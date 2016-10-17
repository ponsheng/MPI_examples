#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "bmp.h"

using namespace std;

//©wžq¥­·Æ¹BºâªºŠžŒÆ
#define NSmooth 1000

/*********************************************************/
/*ÅÜŒÆ«Å§i¡G                                             */
/*  bmpHeader    ¡G BMPÀÉªºŒÐÀY                          */
/*  bmpInfo      ¡G BMPÀÉªºžê°T                          */
/*  **BMPSaveData¡G ÀxŠs­n³QŒg€Jªº¹³¯Àžê®Æ               */
/*  **BMPData    ¡G ŒÈ®ÉÀxŠs­n³QŒg€Jªº¹³¯Àžê®Æ           */
/*********************************************************/
BMPHEADER bmpHeader;                        
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;                                               
RGBTRIPLE **BMPData = NULL;                                                   

/*********************************************************/
/*šçŒÆ«Å§i¡G                                             */
/*  readBMP    ¡G Åªšú¹ÏÀÉ¡AšÃ§â¹³¯Àžê®ÆÀxŠsŠbBMPSaveData*/
/*  saveBMP    ¡G Œg€J¹ÏÀÉ¡AšÃ§â¹³¯Àžê®ÆBMPSaveDataŒg€J  */
/*  swap       ¡G ¥æŽ«€G­Ó«üŒÐ                           */
/*  **alloc_memory¡G °ÊºA€À°t€@­ÓY * X¯x°}               */
/*********************************************************/
int readBMP( char *fileName);        //read file
int saveBMP( char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory( int Y, int X );        //allocate memory

int main(int argc,char *argv[])
{
/*********************************************************/
/*ÅÜŒÆ«Å§i¡G                                             */
/*  *infileName  ¡G ÅªšúÀÉŠW                             */
/*  *outfileName ¡G Œg€JÀÉŠW                             */
/*  startwtime   ¡G °O¿ý¶}©l®É¶¡                         */
/*  endwtime     ¡G °O¿ýµ²§ô®É¶¡                         */
/*********************************************************/
	char *infileName = "input.bmp";
     	char *outfileName = "output2.bmp";
	double startwtime = 0.0, endwtime=0;

	MPI_Init(&argc,&argv);
	
	//°O¿ý¶}©l®É¶¡
	startwtime = MPI_Wtime();

	//ÅªšúÀÉ®×
        if ( readBMP( infileName) )
                cout << "Read file successfully!!" << endl;
        else 
                cout << "Read file fails!!" << endl;

	//°ÊºA€À°t°OŸÐÅéµ¹ŒÈŠsªÅ¶¡
        BMPData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);

        //¶iŠæŠhŠžªº¥­·Æ¹Bºâ
	for(int count = 0; count < NSmooth ; count ++){
		//§â¹³¯Àžê®Æ»PŒÈŠs«üŒÐ°µ¥æŽ«
		swap(BMPSaveData,BMPData);
		//¶iŠæ¥­·Æ¹Bºâ
		for(int i = 0; i<bmpInfo.biHeight ; i++)
			for(int j =0; j<bmpInfo.biWidth ; j++){
				/*********************************************************/
				/*³]©w€W€U¥ª¥k¹³¯ÀªºŠìžm                                 */
				/*********************************************************/
				int Top = i>0 ? i-1 : bmpInfo.biHeight-1;
				int Down = i<bmpInfo.biHeight-1 ? i+1 : 0;
				int Left = j>0 ? j-1 : bmpInfo.biWidth-1;
				int Right = j<bmpInfo.biWidth-1 ? j+1 : 0;
				/*********************************************************/
				/*»P€W€U¥ª¥k¹³¯À°µ¥­§¡¡AšÃ¥|±Ë€­€J                       */
				/*********************************************************/
				BMPSaveData[i][j].rgbBlue =  (double) (BMPData[i][j].rgbBlue+BMPData[Top][j].rgbBlue+BMPData[Down][j].rgbBlue+BMPData[i][Left].rgbBlue+BMPData[i][Right].rgbBlue)/5+0.5;
				BMPSaveData[i][j].rgbGreen =  (double) (BMPData[i][j].rgbGreen+BMPData[Top][j].rgbGreen+BMPData[Down][j].rgbGreen+BMPData[i][Left].rgbGreen+BMPData[i][Right].rgbGreen)/5+0.5;
				BMPSaveData[i][j].rgbRed =  (double) (BMPData[i][j].rgbRed+BMPData[Top][j].rgbRed+BMPData[Down][j].rgbRed+BMPData[i][Left].rgbRed+BMPData[i][Right].rgbRed)/5+0.5;
			}
	}
 
 	//Œg€JÀÉ®×
        if ( saveBMP( outfileName ) )
                cout << "Save file successfully!!" << endl;
        else
                cout << "Save file fails!!" << endl;
	
	//±ošìµ²§ô®É¶¡¡AšÃŠL¥X°õŠæ®É¶¡
        endwtime = MPI_Wtime();
    	cout << "The execution time = "<< endwtime-startwtime <<endl ;

 	free(BMPData);
 	free(BMPSaveData);
 	MPI_Finalize();

        return 0;
}

/*********************************************************/
/* Åªšú¹ÏÀÉ                                              */
/*********************************************************/
int readBMP(char *fileName)
{
	//«Ø¥ß¿é€JÀÉ®×ª«¥ó	
        ifstream bmpFile( fileName, ios::in | ios::binary );
 
        //ÀÉ®×µLªk¶}±Ò
        if ( !bmpFile ){
                cout << "It can't open file!!" << endl;
                return 0;
        }
 
        //ÅªšúBMP¹ÏÀÉªºŒÐÀYžê®Æ
    	bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );
 
        //§PšM¬O§_¬°BMP¹ÏÀÉ
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
 
        //ÅªšúBMPªºžê°T
        bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );
        
        //§PÂ_Šì€ž²`«×¬O§_¬°24 bits
        if ( bmpInfo.biBitCount != 24 ){
                cout << "The file is not 24 bits!!" << endl;
                return 0;
        }

        //­×¥¿¹Ï€ùªºŒe«×¬°4ªº­¿ŒÆ
        while( bmpInfo.biWidth % 4 != 0 )
        	bmpInfo.biWidth++;

        //°ÊºA€À°t°OŸÐÅé
        BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
        
        //Åªšú¹³¯Àžê®Æ
    	//for(int i = 0; i < bmpInfo.biHeight; i++)
        //	bmpFile.read( (char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE));
	bmpFile.read( (char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);
	
        //Ãö³¬ÀÉ®×
        bmpFile.close();
 
        return 1;
 
}
/*********************************************************/
/* ÀxŠs¹ÏÀÉ                                              */
/*********************************************************/
int saveBMP( char *fileName)
{
 	//§PšM¬O§_¬°BMP¹ÏÀÉ
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
        
 	//«Ø¥ß¿é¥XÀÉ®×ª«¥ó
        ofstream newFile( fileName,  ios:: out | ios::binary );
 
        //ÀÉ®×µLªk«Ø¥ß
        if ( !newFile ){
                cout << "The File can't create!!" << endl;
                return 0;
        }
 	
        //Œg€JBMP¹ÏÀÉªºŒÐÀYžê®Æ
        newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );

	//Œg€JBMPªºžê°T
        newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );

        //Œg€J¹³¯Àžê®Æ
        //for( int i = 0; i < bmpInfo.biHeight; i++ )
        //        newFile.write( ( char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE) );
        newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );

        //Œg€JÀÉ®×
        newFile.close();
 
        return 1;
 
}


/*********************************************************/
/* €À°t°OŸÐÅé¡GŠ^¶Ç¬°Y*Xªº¯x°}                           */
/*********************************************************/
RGBTRIPLE **alloc_memory(int Y, int X )
{        
	//«Ø¥ßªø«×¬°Yªº«üŒÐ°}ŠC
        RGBTRIPLE **temp = new RGBTRIPLE *[ Y ];
	RGBTRIPLE *temp2 = new RGBTRIPLE [ Y * X ];
        memset( temp, 0, sizeof( RGBTRIPLE ) * Y);
        memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );

	//¹ïšC­Ó«üŒÐ°}ŠCžÌªº«üŒÐ«Å§i€@­Óªø«×¬°Xªº°}ŠC 
        for( int i = 0; i < Y; i++){
                temp[ i ] = &temp2[i*X];
        }
 
        return temp;
 
}
/*********************************************************/
/* ¥æŽ«€G­Ó«üŒÐ                                          */
/*********************************************************/
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}

