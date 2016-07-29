#include "VerificadorVagas.h"
#include "SegmentadorVagas.h"


Mat modelo;

VerificadorVagas::VerificadorVagas()
{
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

	absdiff(a, b, D);
	//imshow("D", D);
	//threshold(D, D, lm, 255, THRESH_BINARY);
	//imshow("Absoluta", D);
	BinarizacomHist(D, porcentagem, lm);


	//imshow("DBin", D);
	return D;

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


void VerificadorVagas::Iniciar(VideoCapture video, vector<Vaga> vagas, Rect roiModelo, Rect roiVagas)
{
	Mat indicacoes;
	Mat primeiro;
	Mat quadro;
	Mat fundo;
	Mat fundoAnterior;
	video.read(primeiro);
	indicacoes = Mat::zeros(primeiro.rows, primeiro.cols, CV_32FC3);
	cvtColor(primeiro, primeiro, CV_BGR2GRAY);
	modelo = primeiro(roiModelo);



	for (int j = 0; j < vagas.size(); j++) {

		//DeterminaOcupacao(primeiro(vagas[j].limites), j);
		vagas[j].ocupada = false;
	}

	primeiro.copyTo(fundo);
	//primeiro.copyTo(fundoAnterior);

	Mat quadroAnterior;

	vector<Vaga> vagasDin = vagas;

	while (video.isOpened()) {
		//Mat quadro;
		if(!video.read(quadro))
		{
			break;
		}
		else 
		{
			modelo = quadro(roiModelo);
			cvtColor(quadro, quadro, CV_BGR2GRAY);
			if (!quadroAnterior.empty())
			{
				Mat D, DFundo;
				D = Diferenca(quadro, quadroAnterior, 1, 50);
				dilate(D, D, Mat::ones(10, 10, CV_8UC1), Point(-1, -1), 5);

				fundo = AtualizaFundo(fundo, D, quadro);

				imshow("Fundo", fundo);

				/*
				absdiff(fundo, fundoAnterior, DFundo);

				imshow("DFundo", DFundo);

				fundo.copyTo(fundoAnterior);
				*/
				/*
				Mat prev;
				fundo.copyTo(prev);
				SegmentadorVagas sv;
				vagasDin = sv.Segmentar(fundo(roiVagas));
				for each (Vaga v in vagasDin)
				{
					int x = roiVagas.x + v.limites.x;
					int y = roiVagas.y + v.limites.y;
					Rect r = Rect(x, y, v.limites.width, v.limites.height);

					rectangle(prev, r, Scalar(255, 255, 0), 1);

				}
				*/

				VerificaEstado(quadro, fundo, vagas);
				//AtualizaIndicacoes(indicacoes, vagas);
				//imshow("Vagas", prev);
				imshow("Video", quadro);
				imshow("Indicacoes", indicacoes);
				waitKey(1);
			}

			quadro.copyTo(quadroAnterior);

		}


	}

}

void VerificadorVagas::Iniciar(vector<Mat> imagens, vector<Vaga> vagas)
{
	Mat indicacoes;

	for (int i = 0; i < imagens.size(); i++) {
		if (i == 0) {
			indicacoes = Mat::zeros(imagens[0].rows, imagens[0].cols, CV_32FC3);
			for (int j = 0; j < vagas.size(); j++) {
				DeterminaOcupacao(imagens[0](vagas[j].limites),j);
			}
		}
		else {
			cout << "Momento  " << i << endl;

			VerificaEstado(imagens[i], imagens[i-1], vagas);
			AtualizaIndicacoes(indicacoes, vagas);
			//imshow("Antes", imagens[i - 1]);
			imshow("Depois", imagens[i]);
			imshow("Indicacoes", indicacoes);
			waitKey(33);
		}

	}

}

void VerificadorVagas::AtualizaIndicacoes(Mat &indicacoes, vector<Vaga> &vagas)
{
	indicacoes = Mat::zeros(indicacoes.size(), indicacoes.type());
	for (int i = 0; i < vagas.size(); i++) {
		Scalar cor = Scalar(255, 0, 0);
		if (vagas[i].ocupada) {
			cor = Scalar(0, 0, 255);
		}
		//circle(indicacoes, vagas[i].limites., 1, cor);
		rectangle(indicacoes, vagas[i].limites, cor, -1);
	}
}

void VerificadorVagas::VerificaEstado(Mat quadro, Mat fundo,  vector<Vaga> &vagas)
{

	for (int i = 0; i < vagas.size(); i++) {
		Mat imgVaga = quadro(vagas[i].limites);
		//vagas[i].ocupada = DeterminaOcupacao(imgVaga);
		vagas[i].ocupada = ComparaComFundo(fundo, quadro, vagas[i], i);
	}

	//waitKey(0);

}





bool VerificadorVagas::ComparaComFundo(Mat fundo, Mat quadro, Vaga &vaga, int index) {
	
	bool retorno = vaga.ocupada;

	Mat vagaQuadro = quadro(vaga.limites);
	Mat vagaFundo = fundo(vaga.limites);

	Mat D;
	absdiff(vagaQuadro, vagaFundo, D);
	threshold(D, D, 1, 255, THRESH_BINARY);

	Mat erosor = getStructuringElement(MORPH_RECT, cv::Size(3, 3));
	Mat dilatador = getStructuringElement(MORPH_RECT, cv::Size(5, 5));

	erode(D, D, erosor, Point(0, 0), 2);
	dilate(D, D, dilatador, Point(0, 0), 2);

	imshow("D", D);

	
	int tamD = D.rows * D.cols;

	retorno = DeterminaOcupacao(vagaQuadro, index);

	/*
	if (contaBrancos(D) >= tamD*0.2f){
		//return DeterminaOcupacao(vagaQuadro, index);
		vaga.olhando = true;
	}
	else {
		//cout << vaga.olhando << endl;
		if (vaga.olhando == true) {
			cout << "Movimento na vaga " << index << endl;
			retorno = DeterminaOcupacao(vagaQuadro, index);
			vaga.olhando = false;
		}

	}
	*/
	return retorno;




}

int S(int a) {
	if (a >= 0) return 1;
	return 0;

}

Mat normalizaHist(Mat histograma, int niveis) {

	Mat normalizado;
	normalizado = Mat::zeros(histograma.size(), histograma.type());
	int total = 0;

	for (int i = 0; i < niveis; i++) {
		total += histograma.at<float>(i);
	}

	for (int k = 0; k < niveis; k++) {
		normalizado.at<float>(k) = 100 * histograma.at<float>(k) / total;
	}
	


	return normalizado;
}


Mat LBP_OJALA(Mat vaga, int index)
{

	Mat valoresLBP = Mat(vaga.rows, vaga.cols, CV_8UC1);

	unsigned char ant = -1;
	int transicoes = 0;
	int LBP = 0;
	int potencia;

	int R = 5;
	int P;
	/*
	for (int c = 0; c < R; c++) {
		P += 8 * (R - c);
	}
	*/

	for (int i = 1; i < vaga.rows - 1; i++) {
		for (int j = 1; j < vaga.cols-1; j++) {

			unsigned char gc = vaga.at<unsigned char>(i, j);
			LBP = 0;
			ant = -1;
			transicoes = 0;
			P = 0;

			for (int x = -R; x <= R; x++) {
				for (int y = -R; y <= R; y++) {
					if (x != 0 || y != 0) {
						if (x*x + y*y == R*R) {
							int pX = i + x;
							int pY = j + y;
							if (pX >= 0 && pX < vaga.rows && pY >= 0 && pY < vaga.cols) {
								unsigned char gi = vaga.at<unsigned char>(pX, pY);
								int v = S(gi - gc);
								LBP += v;
								if (ant != -1 && ant != v) {
									transicoes++;
								}
								ant = v;
								P++;
							}
						}

					}
				}
			}

			//cout << "LBP do ponto " << i << "," << j << "= " << LBP << endl;
			if (transicoes > 2) {
				LBP = P + 1;
			}
			valoresLBP.at<unsigned char>(i, j) = LBP;
		}
	}
	Mat histograma;
	int tamHist = 8 * R + 2;
	float range[] = { 0, tamHist };
	const float* rangeHist = { range };
	int hist_w = tamHist; int hist_h = 256;
	int bin_w = cvRound((double)hist_w / tamHist);
	Mat histogramaIMG(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

	calcHist(&valoresLBP, 1, 0, Mat(), histograma, 1, &tamHist, &rangeHist, true, false);
	histograma = normalizaHist(histograma, tamHist);


	for (int i = 0; i < tamHist; i++)
	{
		//cout << "i:" << i << "  numero:" << histograma.at<float>(i) << endl;
		line(histogramaIMG, Point(bin_w*(i - 1), hist_h),
			Point(bin_w*(i), hist_h - cvRound(histograma.at<float>(i)) * 5),
			Scalar(255, 0, 0), 2, 8, 0);


	}



	ostringstream oss;
	oss << "LBP da vaga " << index;

	imshow(oss.str(), vaga);


	//cout << "LBP da vaga "<< index << "  " << LBP;
	return histograma;

}

Mat LBP(Mat vaga, int index)
{


	Mat valoresLBP = Mat(vaga.rows, vaga.cols, CV_8UC1);

	unsigned char ant = -1;
	int transicoes = 0;
	int LBP = 0;
	int potencia;

	for (int i = 0; i < vaga.rows; i++) {
		for (int j = 0; j < vaga.cols; j++) {
			
			unsigned char gc = vaga.at<unsigned char>(i, j);
			LBP = 0;
			potencia = 0;
			
			for (int x = -1; x <= 1; x++) {
				for (int y = -1; y <= 1; y++) {
					if (x != 0 || y != 0) {
						int pX = i + x;
						int pY = j + y;
						if (pX >= 0 && pX < vaga.rows && pY >= 0 && pY < vaga.cols) {
							unsigned char gi = vaga.at<unsigned char>(pX, pY);
							int v = S(gi - gc);
							//cout << potencia << endl;
							LBP += v * pow(2.0f,potencia);

							potencia++;
						}
					}
				}
			}

			//cout << "LBP do ponto " << i << "," << j << "= " << LBP << endl;
			valoresLBP.at<unsigned char>(i, j) = LBP;
		}
	}
	Mat histograma;
	int tamHist = 256;
	float range[] = { 0, 256 };
	const float* rangeHist = { range };
	int hist_w = 256; int hist_h = 256;
	int bin_w = cvRound((double)hist_w / tamHist);
	Mat histogramaIMG(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	
	calcHist(&valoresLBP, 1, 0, Mat(), histograma, 1, &tamHist, &rangeHist, true, false);
	histograma = normalizaHist(histograma, tamHist);


	for (int i = 0; i < tamHist; i++)
	{
		//cout << "i:" << i << "  numero:" << histograma.at<float>(i) << endl;
		line(histogramaIMG, Point(bin_w*(i - 1), hist_h),
			Point(bin_w*(i), hist_h - cvRound(histograma.at<float>(i)) * 5),
			Scalar(255, 0, 0), 2, 8, 0);


	}



	ostringstream oss;
	oss << "LBP da vaga " << index;

	imshow(oss.str(), histogramaIMG);


	//cout << "LBP da vaga "<< index << "  " << LBP;
	return histograma;
	

}

Mat ILBP(Mat vaga, int index)
{


	Mat valoresLBP = Mat(vaga.rows, vaga.cols, CV_32FC1);
	

	unsigned char ant = -1;
	int transicoes = 0;
	int LBP = 0;
	int potencia;
	float mediaVizinhanca;
	long int LBPTotal = 0;

	for (int i = 1; i < vaga.rows-1; i++) {
		for (int j = 1; j < vaga.cols-1; j++) {
			LBP = 0;
			potencia = 0;
			mediaVizinhanca = 0;

			for (int x = -1; x <= 1; x++) {
				for (int y = -1; y <= 1; y++) {
					int pX = i + x;
					int pY = j + y;
					mediaVizinhanca += vaga.at<unsigned char>(pX, pY);
				}
			}

			mediaVizinhanca = mediaVizinhanca / 9;

			unsigned char gi;
			gi = vaga.at<unsigned char>(i-1,j-1);
			LBP += S(gi - mediaVizinhanca) * pow(2.0f, 8);

			gi = vaga.at<unsigned char>(i, j - 1);
			LBP += S(gi - mediaVizinhanca) * pow(2.0f, 7);

			gi = vaga.at<unsigned char>(i + 1, j - 1);
			LBP += S(gi - mediaVizinhanca) * pow(2.0f, 6);

			gi = vaga.at<unsigned char>(i + 1, j);
			LBP += S(gi - mediaVizinhanca) * pow(2.0f, 5);

			gi = vaga.at<unsigned char>(i + 1, j + 1);
			LBP += S(gi - mediaVizinhanca) * pow(2.0f, 4);

			gi = vaga.at<unsigned char>(i, j + 1);
			LBP += S(gi - mediaVizinhanca) * pow(2.0f, 3);

			gi = vaga.at<unsigned char>(i - 1, j + 1);
			LBP += S(gi - mediaVizinhanca) * pow(2.0f, 2);

			gi = vaga.at<unsigned char>(i - 1, j);
			LBP += S(gi - mediaVizinhanca) * pow(2.0f, 1);

			gi = vaga.at<unsigned char>(i, j);
			LBP += S(gi - mediaVizinhanca);


			/*
			for (int x = -1; x <= 1; x++) {
				for (int y = -1; y <= 1; y++) {
					int pX = i + x;
					int pY = j + y;
				
					unsigned char gi = vaga.at<unsigned char>(pX, pY);
					int v = S(gi - mediaVizinhanca);
					//cout << potencia << endl;
					LBP += v * pow(2.0f, potencia);

					potencia++;
				}
			}
			*/
			//cout << "LBP do ponto " << i << "," << j << "= " << LBP << endl;
			LBPTotal += LBP;
			
			valoresLBP.at<float>(i, j) = LBP;
		}
	}

	Mat histograma;
	int tamHist = 512;
	float range[] = { 0, 512 };
	const float* rangeHist = { range };
	
	int hist_w = 512; int hist_h = 256;
	int bin_w = cvRound((double)hist_w / tamHist);
	Mat histogramaIMG(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

	calcHist(&valoresLBP, 1, 0, Mat(), histograma, 1, &tamHist, &rangeHist, true, false);
	histograma = normalizaHist(histograma, tamHist);


	for (int i = 0; i < tamHist; i++)
	{
		//cout << "i:" << i << "  numero:" << histograma.at<float>(i) << endl;
		line(histogramaIMG, Point(bin_w*(i - 1), hist_h),
			Point(bin_w*(i), hist_h - cvRound(histograma.at<float>(i)) * 5),
			Scalar(255, 0, 0), 2, 8, 0);


	}



	stringstream oss;
	oss << "Vaga " << index;
	//imshow(oss.str(), vaga);
	oss << "ILBP";


	imshow(oss.str(), histogramaIMG);
	

	cout << "LBP da vaga " << index << "  " << LBPTotal << endl;
	
	return histograma;


}

long float GLCM(Mat vaga, int index) {

	Mat matGLCM = Mat::zeros(256, 256, CV_32FC1);
	for (int i = 0; i < vaga.rows - 1; i++) {
		for (int j = 0; j < vaga.cols; j++) {
			unsigned char ponto = vaga.at<unsigned char>(i, j);
			unsigned char aDireita = vaga.at<unsigned char>(i + 1, j);

			matGLCM.at<float>(ponto, aDireita)++;
		}
	}

	long float contraste = 0;
	long float energia = 0;
	long float homogeneidade = 0;

	for (int i = 0; i < matGLCM.rows; i++) {
		for (int j = 0; j < matGLCM.cols; j++) {
			
			float v = matGLCM.at<float>(i, j);
			//cout << v << endl;
			contraste += (abs(i - j)*abs(i - j)*v);
			energia += v*v;
			homogeneidade += v / (1 + abs(i - j));


		}
	}
	


	cout <<"Vaga "<<index<<" Contraste = " << contraste << " \n Energia = " << energia << " \n Homogeneidade = " << homogeneidade << endl;


	return contraste / 1000;

}


bool VerificadorVagas::DeterminaOcupacao(Mat vaga, int index) {


	/*
	Mat canny;
	//Canny(vaga, canny, 75, 150);
	//ILBP(vaga, index);
	//LBP_OJALA(vaga, index);
	//cout << GLCM(vaga, index)<<"  "<< index;
	//waitKey(0);

	//cout << "Brancos antes na vaga " << indexVaga << ": " << contaBrancos(cannyA) << "   Brancos depois: " << contaBrancos(cannyB) << endl;
	//cout << "  " << (contaBrancos(canny) > 500) << endl;

	//return contaBrancos(canny) > 500;
	/*
	Mat histModelo = LBP_OJALA(modelo, 99);
	Mat histVaga = LBP_OJALA(vaga, index);
	if (index != 0) {
		//cout << "Valor comparacao:  " << compareHist(histModelo, histVaga, CV_COMP_INTERSECT) << endl;
	}
	*/
		
	Mat vagaEQ;

	stringstream oss;
	oss << "Vaga " << index;

	equalizeHist(vaga, vagaEQ);

	imshow(oss.str(), vaga);

	oss << "Equalizada";

	imshow(oss.str(), vagaEQ);

	//ILBP(vagaEQ, index);
	GLCM(vagaEQ, index);

	return false;
	


}

int VerificadorVagas::contaBrancos(Mat a) {
	int zeros = 0;
	vector<Mat> avector;

	split(a, avector);
	for (int i = 0; i<avector.size(); i++) {
		zeros += countNonZero(avector[i]);
	}
	merge(avector, a);


	return zeros;
}
