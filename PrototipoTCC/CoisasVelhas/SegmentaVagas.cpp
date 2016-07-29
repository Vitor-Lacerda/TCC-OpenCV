#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace cv;
using namespace std;

float PicoHistograma(Mat histograma, int tamHist){
	float pico = 0;
	for(int i =0;i<tamHist;i++){
		if(histograma.at<float>(i) > pico){
			pico = histograma.at<float>(i);
		}
	}

	return pico;

}

float LimiarHistograma(Mat histograma, int tamHist, float pico, float porcentagem){
	
	float limiar = pico * (porcentagem/100);
	int cont = 0;
	for(int i = 0;i<tamHist;i++){
		if(histograma.at<float>(i) < limiar){
			cont++;
			if(cont>=3){
				limiar = i;
				break;
			}
		}
	}


	
	return limiar;
}

void BinarizacomHist(Mat &m, float porcentagem, int limiarMinimo){

	Mat histograma;
	int tamHist = 256;
	float range[] = { 0, 256 } ;
	const float* rangeHist = { range };

	threshold(m,m, limiarMinimo, 0, THRESH_TOZERO);

	calcHist(&m,1,0,Mat(),histograma,1,&tamHist,&rangeHist,true,true);

	//Desenhar o histograma na tela
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound( (double) hist_w/tamHist );
	Mat histogramaIMG( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	for( int i = 1; i < tamHist; i++ )
	{
      line( histogramaIMG, Point( bin_w*(i-1), hist_h - cvRound(histograma.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(histograma.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );


	}
	imshow("Histograma", histogramaIMG);

	//Encontra o maior valor do histograma
	float pico = PicoHistograma(histograma, tamHist);
	float limiar = LimiarHistograma(histograma,tamHist,pico,porcentagem);
	threshold(m,m,limiar, 255, CV_THRESH_BINARY);

}


int main( int argc, char** argv )
{

    Mat image;
    image = imread("../estacionamento.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	if( image.empty() )
    {
        cout << "Could not open or find the image" << std::endl ;
		waitKey(0);
        return -1;
    }
    namedWindow( "Imagem", WINDOW_AUTOSIZE );
    imshow( "Imagem", image );

	Mat imageBinarizada;
	image.copyTo(imageBinarizada);

	BinarizacomHist(imageBinarizada, 0.01, 180);

	imshow("Binarizada", imageBinarizada);
	
	Mat imageClosing;
	imageBinarizada.copyTo(imageClosing);
	
	Mat structuring = getStructuringElement(MORPH_RECT, cv::Size(15,3));

	dilate(imageClosing, imageClosing, structuring, Point(0,0), 5);
	erode(imageClosing, imageClosing, structuring, Point(0,0), 5);

	bitwise_not(imageBinarizada, imageBinarizada);
	bitwise_and(imageBinarizada, imageClosing, imageClosing);

	imshow("Closing", imageClosing);

	threshold(imageClosing,imageClosing,1, 255, CV_THRESH_BINARY_INV);
	Mat imageSobreposta;
	image.copyTo(imageSobreposta);

	bitwise_and(imageClosing, imageSobreposta, imageSobreposta);
	
	imshow("Sobreposta", imageSobreposta);



	


    waitKey(0);
    return 0;
}
