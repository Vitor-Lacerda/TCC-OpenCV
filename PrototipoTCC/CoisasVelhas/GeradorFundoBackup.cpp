#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace cv;
using namespace std;

struct Rastro {
	Point inicio;
	Point fim;
	Point fimAnterior;
	int repeticoes;
	Scalar cor;

};

/*Uso pra gerar um fundo inicial*/
void MediaVariancia(int quadros, char* nomevid, Mat &mediaS, Mat &varianciaS) {

	Mat media;
	Mat variancia;
	Mat frame;

	VideoCapture vid(nomevid);

	vid.read(frame);
	media = Mat::zeros(frame.rows, frame.cols, frame.type());
	for (int i = 0; i<quadros; i++) {
		add(frame / quadros, media, media);
		vid.read(frame);

	}

	variancia = Mat::zeros(media.rows, media.cols, media.type());

	media.copyTo(mediaS);
	variancia.copyTo(varianciaS);

}

float PicoHistograma(Mat histograma, int tamHist) {
	float pico = 0;
	for (int i = 0; i<tamHist; i++) {
		if (histograma.at<float>(i) > pico) {
			pico = histograma.at<float>(i);
		}
	}

	return pico;

}

float LimiarHistograma(Mat histograma, int tamHist, float pico, float porcentagem) {

	float limiar = pico * (porcentagem / 100);
	int cont = 0;
	for (int i = 0; i<tamHist; i++) {
		if (histograma.at<float>(i) < limiar) {
			cont++;
			if (cont >= 3) {
				limiar = i;
				break;
			}
		}
	}

	/*float limiar = 0;
	float valorTotal = sum(histograma)[0];
	float somaTotal = 0;

	for(int i = 0;i<tamHist;i++){
	if(somaTotal < valorTotal*(100-porcentagem)/100){
	somaTotal += histograma.at<float>(i);
	}
	else{
	limiar = i;
	break;
	}
	}*/


	return limiar;
}

void BinarizacomHist(Mat &m, float porcentagem, int limiarMinimo) {

	Mat histograma;
	int tamHist = 256;
	float range[] = { 0, 256 };
	const float* rangeHist = { range };

	threshold(m, m, limiarMinimo, 0, THRESH_TOZERO);

	calcHist(&m, 1, 0, Mat(), histograma, 1, &tamHist, &rangeHist, true, true);

	//Desenhar o histograma na tela
	/*
	int hist_w = 300; int hist_h = 160;
	int bin_w = cvRound( (double) hist_w/tamHist );
	Mat histogramaIMG( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	for( int i = 1; i < tamHist; i++ )
	{
	line( histogramaIMG, Point( bin_w*(i-1), hist_h - cvRound(histograma.at<float>(i-1)*10) ) ,
		Point( bin_w*(i), hist_h - cvRound(histograma.at<float>(i)*10) ), Scalar( 255, 0, 0), 2, 8, 0  );

	}
	imshow("Histograma", histogramaIMG);
	*/

	//Encontra o maior valor do histograma
	float pico = PicoHistograma(histograma, tamHist);
	float limiar = LimiarHistograma(histograma, tamHist, pico, porcentagem);
	threshold(m, m, limiar, 255, CV_THRESH_BINARY);

}

Mat Diferenca(Mat a, Mat b, float porcentagem, int lm) {
	Mat D;
	vector<Mat> Dvector;
	
	absdiff(a, b, D);
	//imshow("D", D);
	//threshold(D, D, lm, 255, THRESH_BINARY);

	split(D, Dvector);

	//Faz isso porque a imagem tem 3 canais
	for (int i = 0; i<Dvector.size(); i++) {
		BinarizacomHist(Dvector[i], porcentagem, lm);
	}

	merge(Dvector, D);

	//imshow("DBin", D);
	return D;

}

int contaZeros(Mat a) {
	int zeros = 0;
	vector<Mat> avector;

	split(a, avector);
	for (int i = 0; i<avector.size(); i++) {
		zeros += countNonZero(avector[i]);
	}
	merge(avector, a);


	return zeros;
}

/*Se for diferente do quadro anterior, mostra o que tinha no fundo antes,
se for igual, mostra o que tem no quadro atual */
Mat AtualizaFundo(Mat ant, Mat mascara, Mat quadro) {

	Mat fundo;
	Mat temp;
	Mat media;

	float prop = 0.8;

	//Temp e a imagem que tem o conteudo do fundo anterior na regiao marcada pela mascara
	bitwise_and(ant, mascara, temp);
	
	//Inverte a mascara
	bitwise_not(mascara, mascara);
	//Faz uma "media" dos valores do fundo anterior com os do quadro atual. Mudar prop muda o peso de cada um
	add((1 - prop)*ant, (prop)*quadro, media);
	//Apaga de media os valores da regiao marcada pela mascara
	bitwise_and(media, mascara, media);
	//Coloca temp na regiao apagada e coloca tudo no fundo novo.
	add(media, temp, fundo);
	/*
	imshow("Media", media);
	imshow("Mascara", mascara);
	imshow("Temp", temp);
	*/
	return fundo;
}

Mat Branco3Canais(Mat imagem) {
	Mat branca;

	vector<Mat> avector;
	split(imagem, avector);
	avector[0].copyTo(branca);

	//Cria uma imagem que e branca nos pontos onde algum dos canais rgb é branco.
	for (int i = 0; i<avector.size(); i++) {
		threshold(avector[i], avector[i], 0, 255, THRESH_BINARY);
		bitwise_or(branca, avector[i], branca);
	}

	return branca;
}
/*
Mat RegioesDiferenca(Mat fundo, Mat diferenca, float contornoMinimo) {
	Mat rD;

	vector<Mat> avector;
	split(diferenca, avector);


	Mat mancha = Branco3Canais(diferenca);

	imshow("Mancha", mancha);

	//Cria a imagem que mostra a região do fundo atual onde a tem a mancha branca
	split(fundo, avector);
	for (int i = 0; i<avector.size(); i++) {
		bitwise_and(avector[i], mancha, avector[i]);
	}
	
	merge(avector, rD);
	return rD;
}
*/

void AnalisaDiferencasFundo(Mat regioes, Point2f *roi) {

	vector<vector<Point>> contornos;
	vector<Vec4i> hierarquia;
	//vector<Mat> regioesSplit;
	int tamMinimo = 15;

	bitwise_not(regioes, regioes);
	//split(regioes, regioesSplit);
	//Encontra os contornos
	//imshow("Canal 1 Regioes", regioesSplit[0]);
	findContours(regioes, contornos, hierarquia, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	Point2f centro(0, 0);
	
	for (int i = 0; i < contornos.size(); i++) {
		float raio;
		minEnclosingCircle(contornos[i], centro, raio);
		vector<Point2f> pontos;

		for (int i = 0; i < 4; i++) {
			pontos.push_back(roi[i]);
		}
		//Começou fora terminou dentro
		if (pointPolygonTest(pontos, centro, false) >= 0) {
			cout << raio << endl;
			if (raio >= tamMinimo) {
				cout << "Vagas disponíveis -1\n";
			}
		}
		
	}
}


Mat DesenhaHistograma(Mat histograma, int tamHist) {
	int hist_w = 300; int hist_h = 160;
	int bin_w = cvRound((double)hist_w / tamHist);
	Mat histogramaIMG(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	for (int i = 1; i < tamHist; i++)
	{
		line(histogramaIMG, Point(bin_w*(i - 1), hist_h - cvRound(histograma.at<float>(i - 1) * 10)),
			Point(bin_w*(i), hist_h - cvRound(histograma.at<float>(i) * 10)), Scalar(255, 0, 0), 2, 8, 0);

	}

	return histogramaIMG;

}

void HistogramasDiferenca(Mat regioes, Mat fundo, Mat fundoAnterior) {

	Mat histogramaFundo;
	Mat histogramaFundoAnterior;
	int tamHist = 256;
	float range[] = { 0, 256 };
	const float* rangeHist = { range };
	
	Mat fundo2, fundoAnterior2;
	fundo.copyTo(fundo2);
	fundoAnterior.copyTo(fundoAnterior2);
	Mat mask;
	regioes.copyTo(mask);
	
	cvtColor(fundo2, fundo2, CV_BGR2HSV);
	cvtColor(fundoAnterior2, fundoAnterior2, CV_BGR2HSV);
	vector<Mat> Fvector;
	split(fundo2, Fvector);

	vector<Mat> FAvector;
	split(fundoAnterior2, FAvector);

	calcHist(&Fvector[0], 1, 0, mask, histogramaFundo, 1, &tamHist, &rangeHist, true, true);
	calcHist(&FAvector[0], 1, 0, mask, histogramaFundoAnterior, 1, &tamHist, &rangeHist, true, true);

	imshow("HistogramaFundo", DesenhaHistograma(histogramaFundo, tamHist));
	imshow("HistogramaFundoAnterior", DesenhaHistograma(histogramaFundoAnterior, tamHist));


}


int main(int argc, char** argv) {

	Mat media, variancia;
	Mat fundo, quadroAnterior;
	Mat fundoAnterior, diferencaFundos;
	Mat roi;
	VideoCapture vid(argv[1]);

	RotatedRect conjuntoDasVagas = RotatedRect(Point2f(550, 150), Size2f(200, 70), 70);

	int cont = 0;

	MediaVariancia(30, argv[1], media, variancia);
	media.copyTo(fundo);
	fundo.copyTo(fundoAnterior);
	fundo.copyTo(roi);



	while (vid.isOpened()) {
		Mat quadro;
		if (!vid.read(quadro)) {
			break;
		}
		else {
			cont++;
			Mat D;
			if (!quadroAnterior.empty()) {
				D = Diferenca(quadro, quadroAnterior, 1, atoi(argv[2]));
				//Se os quadros não forem exatamente iguais atualiza o fundo
				if (contaZeros(D)>0) {
					Mat estrut;
					estrut = Mat::ones(10, 10, CV_8UC1);
					dilate(D, D, estrut, Point(-1, -1), 5);

					fundo = AtualizaFundo(fundo, D, quadro);
					Point2f vertices[4];
					conjuntoDasVagas.points(vertices);

					for (int i = 0; i < 4; i++) {
						line(roi, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
					}
					
					if (cont > 150) {
						cont = 0;

						diferencaFundos = Diferenca(fundo, fundoAnterior, 1, 50);
						estrut = Mat::ones(3, 3, CV_8UC1);

						erode(diferencaFundos, diferencaFundos, estrut, Point(-1, -1), 1);
						dilate(diferencaFundos,diferencaFundos, estrut, Point(-1, -1), 5);

						Mat regioes = Branco3Canais(diferencaFundos);

						//imshow("DiferencaFundos", diferencaFundos);
						imshow("Regioes",regioes);
						
						
						Mat temp1, temp2;
						fundo.copyTo(temp1);
						fundoAnterior.copyTo(temp2);

						vector<Mat> fvector, favector;
						split(temp1, fvector);
						split(temp2, favector);
						for (int i = 0; i<fvector.size(); i++) {
							bitwise_and(fvector[i], regioes, fvector[i]);
							bitwise_and(favector[i], regioes, favector[i]);
						}

						merge(fvector, temp1);
						merge(favector, temp2);

						imshow("MAscara fundo", temp1);
						imshow("Mascara fundo anterior", temp2);
						
						AnalisaDiferencasFundo(regioes, vertices);
						//HistogramasDiferenca(regioes, fundo, fundoAnterior);

						fundo.copyTo(fundoAnterior);
						waitKey(1);
					}
					
				}
				//imshow("Dilatada", D);
				imshow("Fundo", fundo);
				imshow("Video", quadro);
				imshow("Area das vagas", roi);
				waitKey(1);
			}

			quadro.copyTo(quadroAnterior);
		}
	}
	 
	return 0;
}