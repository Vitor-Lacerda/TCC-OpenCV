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

float DistanciaEuclidiana(Point2f a, Point2f b) {

	Point2f p = b - a;
	return sqrt(p.x*p.x + p.y*p.y);

}


Rastro NovoRastro(Point inicio, Point fim, int seed) {
	Rastro novoRastro;
	RNG rng(seed);
	novoRastro.inicio = inicio;
	novoRastro.fim = fim;
	novoRastro.fimAnterior = fim;
	novoRastro.repeticoes = 0;
	novoRastro.cor = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	return novoRastro;


}


void DesenhaRastro(Mat area, Mat &rastro, vector<Rastro> &rastros) {

	vector<vector<Point>> contornos;
	vector<Vec4i> hierarquia;
	//vector<Mat> areaSplit;
	int tamMinimo = 1000;

	bool crianovo = true;

	bitwise_not(area, area);
	//Pra mexer com um canal só
	//split(area, areaSplit);
	//Encontra os contornos
	findContours(area, contornos, hierarquia, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);


	//Se o contorno for grande o suficiente (?) acha o centro e armazena
	Point2f centro(0, 0);

	for (int i = 0; i<contornos.size(); i++) {
		if (contourArea(contornos[i]) >= tamMinimo) {
			float raio;
			minEnclosingCircle(contornos[i], centro, raio);
			//Se a coloeção de rastros estiver vazia cria um novo
			if (rastros.size() == 0) {
				//cout << "Criou um"<<novoRastro.inicio<<endl;
				rastros.push_back(NovoRastro(centro, centro, rastros.size()));
			}
			//Caso contrario confiro algumas coisas
			else {
				crianovo = true;
				//Se o centro atual estiver perto o suficiente do ponto final de algum rastro ele é o novo final
				for (int j = 0; j < rastros.size(); j++) {
					if (abs(DistanciaEuclidiana(rastros[j].fim, centro)) <= 50) {

						rastros[j].fimAnterior = rastros[j].fim;
						rastros[j].fim = centro;

						crianovo = false;
						//Desenha na imagem
						circle(rastro, centro, 5, rastros[j].cor, -1);
					}

				}
				//Se nao, inicia um rastro novo
				if (crianovo) {
					//cout << "Criou um" << novoRastro.inicio << endl;
					rastros.push_back(NovoRastro(centro, centro, rastros.size()));
				}
			}
		}
	}

}


int ConferePosicao(Rastro rastro, Point2f *roi) {
	vector<Point2f> pontos;

	for (int i = 0; i < 4; i++) {
		pontos.push_back(roi[i]);
	}
	//Começou fora terminou dentro
	if (pointPolygonTest(pontos, rastro.inicio, false) < 0 && pointPolygonTest(pontos, rastro.fim, false) >= 0) {
		cout << "Entrou\n";
		return -1;
	}
	//Começou dentro terminou fora
	if (pointPolygonTest(pontos, rastro.inicio, false) >= 0 && pointPolygonTest(pontos, rastro.fim, false)<0) {
		cout << "Saiu\n";
		return 1;
	}

	return 0;
}

void AnalisaRastros(vector<Rastro> &rastros, Point2f *roi, vector<Point> &fins) {
	for (int j = 0; j < rastros.size(); j++) {
		if (abs(DistanciaEuclidiana(rastros[j].fim, rastros[j].fimAnterior)) < 0.1f) {
			rastros[j].repeticoes++;
			//cout << "Repeticoes "<< j << " = " << rastros[j].repeticoes << endl;
		}
		else {
			rastros[j].fimAnterior = rastros[j].fim;
		}
		if (rastros[j].repeticoes >= 3) {
			int pos = ConferePosicao(rastros[j], roi);
			if (pos<0) {
				//cout << "Apagando rastro " << j << endl;
				fins.push_back(rastros[j].fim);
			}
			else if (pos >0) {
				//Guarda o inicio do rastro
			}
			rastros.erase(rastros.begin() + j);
		}
	}
}

Mat Branco3Canais(Mat imagem) {
	Mat branca;

	vector<Mat> avector;
	split(imagem, avector);
	avector[0].copyTo(branca);

	imshow("1", avector[0]);
	imshow("2", avector[1]);
	imshow("3", avector[2]);

	//Cria uma imagem que e branca nos pontos onde algum dos canais rgb é branco.
	for (int i = 0; i<avector.size(); i++) {
		threshold(avector[i], avector[i], 0, 255, THRESH_BINARY);
		bitwise_or(branca, avector[i], branca);
	}


	imshow("branca", branca);

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
/*
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
*/

void AnalisaDiferencasFundo(Mat regioes, Point2f *roi, vector<Point> &diferencasVagas) {

	vector<vector<Point>> contornos;
	vector<Vec4i> hierarquia;
	//vector<Mat> regioesSplit;
	int tamMinimo = 10;

	bitwise_not(regioes, regioes);
	//split(regioes, regioesSplit);
	//Encontra os contornos
	//imshow("Canal 1 Regioes", regioesSplit[0]);
	findContours(regioes, contornos, hierarquia, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	Point2f centro(0, 0);
	float raio;
	vector<Point2f> pontos;

	for (int i = 0; i < 4; i++) {
		pontos.push_back(roi[i]);
	}


	for (int i = 0; i < contornos.size(); i++) {
		
		minEnclosingCircle(contornos[i], centro, raio);
		//Começou fora terminou dentro
		if (pointPolygonTest(pontos, centro, false) >= 0) {
			cout << raio << endl;
			if (raio >= tamMinimo) {
				diferencasVagas.push_back(centro);
				cout << "Diferença nas Vagas\n";
			}
		}

	}
}

void ComparaRastrosEFundos(vector<Point> &finaisRastros, vector<Point> &diferencasVagas, Mat &rastro) {
	for (int i = 0; i < diferencasVagas.size(); i++) {
		for (int j = 0; j < finaisRastros.size(); j++) {
			/*
			cout << DistanciaEuclidiana(diferencasVagas[i], finaisRastros[j]) << endl;
			circle(rastro, finaisRastros[j], 1, Scalar(255,255,255), -1);
			circle(rastro, diferencasVagas[i], 1, Scalar(0, 0, 0), -1);
			*/
			if (abs(DistanciaEuclidiana(diferencasVagas[i], finaisRastros[j]) < 10.0f)) {
				finaisRastros.erase(finaisRastros.begin() + j);
				diferencasVagas.erase(diferencasVagas.begin() + i);
				cout << "Vagas disponiveis -1\n";
				return;
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
	Mat rastro;
	vector<Rastro> rastros;
	vector<Point> finaisRastros;
	vector<Point> diferencasVagas;

	VideoCapture vid(argv[1]);

	RotatedRect conjuntoDasVagas = RotatedRect(Point2f(550, 150), Size2f(200, 70), 70);

	int cont = 0;

	MediaVariancia(30, argv[1], media, variancia);
	media.copyTo(fundo);
	fundo.copyTo(fundoAnterior);
	fundo.copyTo(rastro);

	Point2f vertices[4];
	conjuntoDasVagas.points(vertices);




	for (int i = 0; i < 4; i++) {
		line(rastro, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
	}



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
					Mat Dfundo, Drastro;
					estrut = Mat::ones(6, 6, CV_8UC1);
					dilate(D, Dfundo, estrut, Point(-1, -1), 5);
					fundo = AtualizaFundo(fundo, Dfundo, quadro);

					/*
					estrut = Mat::ones(3, 3, CV_8UC1);
					erode(D, Drastro, estrut, Point(-1, -1), 1);
					dilate(Drastro, Drastro, estrut, Point(-1, -1), 5);
					imshow("Fechada", Drastro);
					*/

					DesenhaRastro(Branco3Canais(Dfundo), rastro, rastros);
					cont++;
					if (cont > 150) {
						AnalisaRastros(rastros, vertices, finaisRastros);

						diferencaFundos = Diferenca(fundo, fundoAnterior, 1, 50);
						
						estrut = Mat::ones(3, 3, CV_8UC1);
						erode(diferencaFundos, diferencaFundos, estrut, Point(-1, -1), 1);
						dilate(diferencaFundos,diferencaFundos, estrut, Point(-1, -1), 5);
						Mat regioes = Branco3Canais(diferencaFundos);

						//imshow("DiferencaFundos", diferencaFundos);
						//imshow("Fundo Anterior", fundoAnterior);
						imshow("Regioes",regioes);
						HistogramasDiferenca(regioes, fundo, fundoAnterior);
						
						waitKey(0);
						
						/*
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
						*/

						AnalisaDiferencasFundo(regioes, vertices, diferencasVagas);
						ComparaRastrosEFundos(finaisRastros, diferencasVagas, rastro);
						//finaisRastros.clear();
						//diferencasVagas.clear();
						cont = 0;

						//HistogramasDiferenca(regioes, fundo, fundoAnterior);

						fundo.copyTo(fundoAnterior);
					}
					
				}
				//imshow("Dilatada", D);
				imshow("Fundo", fundo);
				imshow("Video", quadro);
				imshow("Rastro", rastro);
				waitKey(1);
			}

			quadro.copyTo(quadroAnterior);
		}
	}
	 
	return 0;
}