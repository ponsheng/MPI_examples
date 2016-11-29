#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <assert.h>
#include "bmp.h"
#include <sys/time.h>
#include <sys/timeb.h>
#include <semaphore.h>

using namespace std;

//定義平滑運算的次數
#ifndef NSmooth
#define NSmooth 1000
#endif

#ifndef NProc
#define NProc 1
#endif

/*********************************************************/
/*變數宣告：                                             */
/*  bmpHeader    ： BMP檔的標頭                          */
/*  bmpInfo      ： BMP檔的資訊                          */
/*  **BMPSaveData： 儲存要被寫入的像素資料               */
/*  **BMPData    ： 暫時儲存要被寫入的像素資料           */
/*********************************************************/
BMPHEADER bmpHeader;
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;
RGBTRIPLE **BMPData = NULL;

int counter_c=0;
int counter_u=0;
//sem_t count_sem = 1;
pthread_mutex_t barrier_mutex_u;
sem_t barrier_sem_u;

pthread_mutex_t barrier_mutex_c;
sem_t barrier_sem_c;

/*********************************************************/
/*函數宣告：                                             */
/*  readBMP    ： 讀取圖檔，並把像素資料儲存在BMPSaveData*/
/*  saveBMP    ： 寫入圖檔，並把像素資料BMPSaveData寫入  */
/*  swap       ： 交換二個指標                           */
/*  **alloc_memory： 動態分配一個Y * X矩陣               */
/*********************************************************/
int readBMP( char *fileName);        //read file
int saveBMP( char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory( int Y, int X );        //allocate memory


long long getSystemTime()
{
	struct timeb t;
	ftime(&t);
	return 1000 * t.time + t.millitm;
}

void barrier_compute(int n)
{
	pthread_mutex_lock(&barrier_mutex_c);
	if(counter_c == NProc -1)
	{
		counter_c = 0;
		//printf("release N=%d\n",n);
		for (int sem = 0; sem < NProc -1 ; sem++)
			sem_post(&barrier_sem_c);
		pthread_mutex_unlock(&barrier_mutex_c);
	}
	else
	{
		++counter_c;
		//printf("wait ");
		pthread_mutex_unlock(&barrier_mutex_c);
		sem_wait(&barrier_sem_c);
	}
}

void barrier_update(int n)
{
    pthread_mutex_lock(&barrier_mutex_u);
    if(counter_u == NProc -1)
    {
        counter_u = 0;
        //printf("release N=%d\n",n);
        for (int sem = 0; sem < NProc -1 ; sem++)
            sem_post(&barrier_sem_u);
        pthread_mutex_unlock(&barrier_mutex_u);
    }
    else
    {
        ++counter_u;
        //printf("wait ");
        pthread_mutex_unlock(&barrier_mutex_u);
        sem_wait(&barrier_sem_u);
    }
}


void *smooth_parallel(void* arg)
{
	//BMPData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
	RGBTRIPLE **local_data;
	long id =(long) arg;
	int start_row = id * (bmpInfo.biHeight/NProc);
	int end_row = (id != NProc -1)? (id +1) * (bmpInfo.biHeight/NProc) -1 : bmpInfo.biHeight -1;
	printf("Thread: %ld start: %d end: %d\n",id,start_row,end_row);

	local_data = alloc_memory( end_row - start_row +1 , bmpInfo.biWidth);

	for(int count = 0; count < NSmooth ; count ++)
	{
		//進行平滑運算
		for(int i = start_row; i<= end_row; i++)
		{
			for(int j =0; j<bmpInfo.biWidth ; j++) {
				int Top = i>0 ? i-1 : bmpInfo.biHeight-1;
				int Down = i<bmpInfo.biHeight-1 ? i+1 : 0;
				int Left = j>0 ? j-1 : bmpInfo.biWidth-1;
				int Right = j<bmpInfo.biWidth-1 ? j+1 : 0;
				int relative_i = i - start_row;

				local_data[relative_i][j].rgbBlue =  (double) (BMPSaveData[i][j].rgbBlue+BMPSaveData[Top][j].rgbBlue+BMPSaveData[Down][j].rgbBlue+BMPSaveData[i][Left].rgbBlue+BMPSaveData[i][Right].rgbBlue)/5+0.5;
				local_data[relative_i][j].rgbGreen =  (double) (BMPSaveData[i][j].rgbGreen+BMPSaveData[Top][j].rgbGreen+BMPSaveData[Down][j].rgbGreen+BMPSaveData[i][Left].rgbGreen+BMPSaveData[i][Right].rgbGreen)/5+0.5;
				local_data[relative_i][j].rgbRed =  (double) (BMPSaveData[i][j].rgbRed+BMPSaveData[Top][j].rgbRed+BMPSaveData[Down][j].rgbRed+BMPSaveData[i][Left].rgbRed+BMPSaveData[i][Right].rgbRed)/5+0.5;
			}
		}
		//wait every thread computing
		barrier_compute(count);
		//fflush(stdout);
		//put data back
		memcpy ( BMPSaveData[start_row], local_data[0], (end_row - start_row +1)*bmpInfo.biWidth * sizeof(RGBTRIPLE) );
		//wait every thread updating data
		barrier_update(count);
		//fflush(stdout);
	}
	free(local_data);
}

int main(int argc,char *argv[])
{
	/*********************************************************/
	/*變數宣告：                                             */
	/*  *infileName  ： 讀取檔名                             */
	/*  *outfileName ： 寫入檔名                             */
	/*  startwtime   ： 記錄開始時間                         */
	/*  endwtime     ： 記錄結束時間                         */
	/*********************************************************/
	char infileName[] = "input.bmp";
	char outfileName[] = "output.bmp";
	long long startwtime = 0, endwtime=0;
	pthread_t* thread_id;


	int ret = sem_init(&barrier_sem_u, 0,0);
	assert(! ret);

	ret = sem_init(&barrier_sem_c, 0,0);
    assert(! ret);

	thread_id = (pthread_t*)malloc( NProc * sizeof(pthread_t));

	//記錄開始時間
	startwtime = getSystemTime();

	//讀取檔案
	if ( readBMP( infileName) )
		cout << "Read file successfully!!" << endl;
	else
		cout << "Read file fails!!" << endl;


	for(long thread = 0; thread < NProc; thread++)
	{
		pthread_create( &thread_id[thread], NULL, smooth_parallel, (void*) thread);
	}

	for(int thread = 0; thread < NProc; thread++)
		pthread_join( thread_id[thread], NULL);


	//寫入檔案
	if ( saveBMP( outfileName ) )
		cout << "Save file successfully!!" << endl;
	else
		cout << "Save file fails!!" << endl;

	//得到結束時間，並印出執行時間
	endwtime = getSystemTime();
	cout << "The execution time = "<< endwtime-startwtime << " ms"  <<endl ;

	free(BMPSaveData);

	return 0;
}

/*********************************************************/
/* 讀取圖檔                                              */
/*********************************************************/
int readBMP(char *fileName)
{
	//建立輸入檔案物件
	ifstream bmpFile( fileName, ios::in | ios::binary );

	//檔案無法開啟
	if ( !bmpFile ) {
		cout << "It can't open file!!" << endl;
		return 0;
	}

	//讀取BMP圖檔的標頭資料
	bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );

	//判決是否為BMP圖檔
	if( bmpHeader.bfType != 0x4d42 ) {
		cout << "This file is not .BMP!!" << endl ;
		return 0;
	}

	//讀取BMP的資訊
	bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );

	//判斷位元深度是否為24 bits
	if ( bmpInfo.biBitCount != 24 ) {
		cout << "The file is not 24 bits!!" << endl;
		return 0;
	}

	//修正圖片的寬度為4的倍數
	while( bmpInfo.biWidth % 4 != 0 )
		bmpInfo.biWidth++;

	//動態分配記憶體
	BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);

	//讀取像素資料
	//for(int i = 0; i < bmpInfo.biHeight; i++)
	//	bmpFile.read( (char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE));
	bmpFile.read( (char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);

	//關閉檔案
	bmpFile.close();

	return 1;

}
/*********************************************************/
/* 儲存圖檔                                              */
/*********************************************************/
int saveBMP( char *fileName)
{
	//判決是否為BMP圖檔
	if( bmpHeader.bfType != 0x4d42 ) {
		cout << "This file is not .BMP!!" << endl ;
		return 0;
	}

	//建立輸出檔案物件
	ofstream newFile( fileName,  ios:: out | ios::binary );

	//檔案無法建立
	if ( !newFile ) {
		cout << "The File can't create!!" << endl;
		return 0;
	}

	//寫入BMP圖檔的標頭資料
	newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );

	//寫入BMP的資訊
	newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );

	//寫入像素資料
	//for( int i = 0; i < bmpInfo.biHeight; i++ )
	//        newFile.write( ( char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE) );
	newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );

	//寫入檔案
	newFile.close();

	return 1;

}


/*********************************************************/
/* 分配記憶體：回傳為Y*X的矩陣                           */
/*********************************************************/
RGBTRIPLE **alloc_memory(int Y, int X )
{
	//建立長度為Y的指標陣列
	RGBTRIPLE **temp = new RGBTRIPLE *[ Y ];
	RGBTRIPLE *temp2 = new RGBTRIPLE [ Y * X ];
	memset( temp, 0, sizeof( RGBTRIPLE ) * Y);
	memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );

	//對每個指標陣列裡的指標宣告一個長度為X的陣列
	for( int i = 0; i < Y; i++) {
		temp[ i ] = &temp2[i*X];
	}

	return temp;

}
/*********************************************************/
/* 交換二個指標                                          */
/*********************************************************/
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}

