#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "bmp.h"

using namespace std;

//定義平滑運算的次數
#ifndef NSmooth
#define NSmooth 1000
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

int main(int argc,char *argv[])
{
/*********************************************************/
/*變數宣告：                                             */
/*  *infileName  ： 讀取檔名                             */
/*  *outfileName ： 寫入檔名                             */
/*  startwtime   ： 記錄開始時間                         */
/*  endwtime     ： 記錄結束時間                         */
/*********************************************************/
	char *infileName = "input.bmp";
    char *outfileName = "output2.bmp";
	double startwtime = 0.0, endwtime=0;
	int comm_size_tmp;
    int my_id;
	
	RGBTRIPLE **LOCALData = NULL;
	RGBTRIPLE **LOCALSaveData = NULL;
	int height;
	int width;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size_tmp);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	const int comm_size = comm_size_tmp;

	//讀取檔案
	if(my_id == 0)
	{
        if ( readBMP( infileName) )
                cout << "Read file successfully!!" << endl;
        else 
                cout << "Read file fails!!" << endl;

	//動態分配記憶體給暫存空間
		printf("bmp info : height:%d   wigth:%d \n\n",bmpInfo.biHeight,bmpInfo.biWidth);
		//記錄開始時間
		startwtime = MPI_Wtime();
		height = bmpInfo.biHeight/comm_size;
		width = bmpInfo.biWidth;
	}
	//Broadcasts height width
	MPI_Bcast(&height, 1, MPI_INT , 0, MPI_COMM_WORLD);
	MPI_Bcast(&width, 1, MPI_INT , 0, MPI_COMM_WORLD);
	printf("%d * %d\n",height,width);
	
	LOCALData = alloc_memory( height+2, width);
	LOCALSaveData = alloc_memory( height+2, width);

	{
		int displs[comm_size];
		int scounts[comm_size];
		for (int i=0; i<comm_size; ++i) { 
        	displs[i] = i*height*width*3; 
        	scounts[i] = height*width*3; 
    	}
		if(my_id ==0)
		MPI_Scatterv( *BMPSaveData, scounts, displs, MPI_BYTE, LOCALData[1]
			, height*width*3, MPI_BYTE, 0, MPI_COMM_WORLD);
		else
		MPI_Scatterv( NULL, scounts, displs, MPI_BYTE, LOCALData[1]
            , height*width*3, MPI_BYTE, 0, MPI_COMM_WORLD);
	}	

    //進行多次的平滑運算
    int my_front = (my_id ==0)? comm_size-1 : my_id-1;
	int my_back = (my_id == comm_size-1)? 0 : my_id+1;
	for(int count = 0; count < NSmooth ; count ++){
		//把像素資料與暫存指標做交換

		MPI_Send(LOCALData[1], width*3, MPI_BYTE, my_front, 0, MPI_COMM_WORLD);
		MPI_Send(LOCALData[height], width*3, MPI_BYTE, my_back, 1, MPI_COMM_WORLD);	
		MPI_Recv(LOCALData[0], width*3, MPI_BYTE, my_front, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(LOCALData[height+1], width*3, MPI_BYTE, my_back, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//進行平滑運算
		for(int i = 1; i<height+1 ; i++)
			for(int j =0; j<width ; j++){
				//設定上下左右像素的位置                                 
				int Top = i-1;//i>0 ? i-1 : height+1;
				int Down = i+1; // i<height-1 ? i+1 : 0;
				int Left = j>0 ? j-1 : width-1;
				int Right = j<width-1 ? j+1 : 0;
				//與上下左右像素做平均，並四捨五入       
				LOCALSaveData[i][j].rgbBlue =  (double) (LOCALData[i][j].rgbBlue+LOCALData[Top][j].rgbBlue+LOCALData[Down][j].rgbBlue+LOCALData[i][Left].rgbBlue+LOCALData[i][Right].rgbBlue)/5+0.5;
				LOCALSaveData[i][j].rgbGreen =  (double) (LOCALData[i][j].rgbGreen+LOCALData[Top][j].rgbGreen+LOCALData[Down][j].rgbGreen+LOCALData[i][Left].rgbGreen+LOCALData[i][Right].rgbGreen)/5+0.5;
				LOCALSaveData[i][j].rgbRed =  (double) (LOCALData[i][j].rgbRed+LOCALData[Top][j].rgbRed+LOCALData[Down][j].rgbRed+LOCALData[i][Left].rgbRed+LOCALData[i][Right].rgbRed)/5+0.5;
			}
		swap(LOCALSaveData,LOCALData);
	}
	swap(LOCALSaveData,LOCALData);
    {
        int displs[comm_size];
        int scounts[comm_size];
        for (int i=0; i<comm_size; ++i) { 
            displs[i] = i*height*width*3; 
            scounts[i] = height*width*3; 
        }
        if(my_id ==0)
        MPI_Gatherv( LOCALData[1], height*width*3, MPI_BYTE
			, *BMPSaveData, scounts, displs, MPI_BYTE, 0, MPI_COMM_WORLD);
        else
        MPI_Gatherv( LOCALData[1], height*width*3, MPI_BYTE
            , NULL, scounts, displs, MPI_BYTE, 0, MPI_COMM_WORLD);
    }


	if(my_id == 0)
	{
	
 	//寫入檔案
        if ( saveBMP( outfileName ) )
                cout << "Save file successfully!!" << endl;
        else
                cout << "Save file fails!!" << endl;
	
	//得到結束時間，並印出執行時間
        endwtime = MPI_Wtime();
    	cout << "The execution time = "<< endwtime-startwtime <<endl ;

 	free(BMPSaveData);
	}
	free(LOCALData);
	free(LOCALSaveData);
 	MPI_Finalize();

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
        if ( !bmpFile ){
                cout << "It can't open file!!" << endl;
                return 0;
        }
 
        //讀取BMP圖檔的標頭資料
    	bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );
 
        //判決是否為BMP圖檔
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
 
        //讀取BMP的資訊
        bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );
        
        //判斷位元深度是否為24 bits
        if ( bmpInfo.biBitCount != 24 ){
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
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
        
 	//建立輸出檔案物件
        ofstream newFile( fileName,  ios:: out | ios::binary );
 
        //檔案無法建立
        if ( !newFile ){
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
        for( int i = 0; i < Y; i++){
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

