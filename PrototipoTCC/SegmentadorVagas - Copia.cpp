#include "SegmentadorVagas.h"



SegmentadorVagas::SegmentadorVagas()
{
}


float SegmentadorVagas::PicoHistograma(Mat histograma, int tamHist) {
	float pico = 0;
	int valor = 0;
	for (int i = 0; i<tamHist; i++) {
		if (histograma.at<float>(i) > pico) {
			pico = histograma.at<float>(i);
			valor = i;
		}
	}

	return valor;

}


float SegmentadorVagas::LimiarHistograma(Mat histograma, int tamHist, float pico, float porcentagem) {


	int quantidadePico;
	int limiar;
	int retorno;

	quantidadePico = histograma.at<float>(pico);
	limiar = quantidadePico * porcentagem / 100;

	//int cont = 0;
	for (int i = 0; i<tamHist; i++) {
		if (histograma.at<float>(i) < limiar && i > pico) {
			//cont++;
			//if (cont >= 0) {
			return i;
			//}
		}
	}



	return pico;
	
}


void SegmentadorVagas::BinarizacomHist(Mat &m, float porcentagem, int limiarMinimo) {

	Mat histograma;
	int tamHist = 256;
	float range[] = { 0, 256 };
	const float* rangeHist = { range };

	threshold(m, m, limiarMinimo, 0, THRESH_TOZERO);

	calcHist(&m, 1, 0, Mat(), histograma, 1, &tamHist, &rangeHist, true, true);

	//Desenhar o histograma na tela
	/*
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / tamHist);
	Mat histogramaIMG(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	for (int i = 1; i < tamHist; i++)
	{
		line(histogramaIMG, Point(bin_w*(i - 1), hist_h - cvRound(histograma.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(histograma.at<float>(i))),
			Scalar(255, 0, 0), 2, 8, 0);


	}
	imshow("Histograma", histogramaIMG);
	*/

	//Encontra o maior valor do histograma
	float pico = PicoHistograma(histograma, tamHist);
	//cout << pico << endl;
	float limiar = LimiarHistograma(histograma, tamHist, pico, porcentagem);
	//cout << limiar << endl;
	threshold(m, m, limiar, 255, CV_THRESH_BINARY);

}

void SegmentadorVagas::GiraImagem(Mat &m) 
{
	Mat girada = Mat(m.cols, m.rows, m.type());
	for (int i = 0; i < m.rows; i++) {
		for (int j = 0; j < m.cols; j++) {

			girada.at<unsigned char>(j, i) = m.at<unsigned char>(i, j);

		}
	}

	girada.copyTo(m);
}

void SegmentadorVagas::OperacoesMorfologicas(Mat &m)
{

	Mat erosao = getStructuringElement(MORPH_RECT, cv::Size(1, 5));
	Mat dilatacao = getStructuringElement(MORPH_RECT, cv::Size(1, 10));

	erode(m, m, erosao, Point(0, 0), 2);
	//imshow("Erodida", m);
	dilate(m, m, dilatacao, Point(0, 0), 5);

}

vector<Vaga> SegmentadorVagas::EncontraVagas(Mat m, bool girou)
{



	vector<Vaga> vetorVagas;
	bool contando = true;
	int altura = m.rows;
	int x = 0;

	for (int i = 0; i < m.cols; i++) {
		if (m.row(0).at<unsigned char>(0, i) == 0 && !contando) {
			x = i;
			contando = true;
		}
		else if(contando && m.row(0).at<unsigned char>(0, i) != 0) {
			
			contando = false;
			if (x != 0) {
				Rect limites;
				if (!girou) {limites = Rect(x, 0, i - x, altura); }
				else { limites = Rect(0, x, altura, i - x); }
				vetorVagas.push_back(Vaga(limites, false));
			}
		}
	} 


	return vetorVagas;

}

vector<Vaga> SegmentadorVagas::Segmentar(Mat img)
{
	Mat bin;
	img.copyTo(bin);
	//cvtColor(img, bin, CV_BGR2GRAY);
	bool girou = false;
	
	if (bin.rows > bin.cols) {
		GiraImagem(bin);
		//GiraImagem(img);
		girou = true;
	}
	BinarizacomHist(bin, 5, 0);
	//imshow("Binarizada", bin);

	OperacoesMorfologicas(bin);
	//imshow("Morfologicas", bin);
	/*
	if (girou) {
		GiraImagem(bin);
	}*/

	return EncontraVagas(bin, girou);

}

