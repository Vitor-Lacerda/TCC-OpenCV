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

Mat Diferenca(Mat a, Mat b, float porcentagem, int lm) {
	Mat D;
	
	absdiff(a, b, D);
	threshold(D, D, lm, 255, THRESH_BINARY);
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

	float prop = 0.5;

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
	
	imshow("Media", media);
	imshow("Mascara", mascara);
	imshow("Temp", temp);
	
	return fundo;
}

float DistanciaEuclidiana(Point2f a, Point2f b) {

	Point2f p = b-a;
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


void Kalman(Mat area, Mat &rastro, vector<Rastro> &rastros) {

	vector<vector<Point>> contornos;
	vector<Vec4i> hierarquia;
	vector<Mat> areaSplit;
	int tamMinimo = 1000;
	bool crianovo = true;

	KalmanFilter KF(2, 2, 0);
	Mat state(2, 1, CV_32F); /* (phi, delta_phi) */
	Mat processNoise(2, 1, CV_32F);
	Mat measurement = Mat::zeros(2, 1, CV_32F);

	//randn(state, Scalar::all(0), Scalar::all(0.1));
	KF.transitionMatrix = (Mat_<float>(2, 2) << 1, 1, 0, 1);

	setIdentity(KF.measurementMatrix);
	setIdentity(KF.processNoiseCov, Scalar::all(1e-5));
	setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
	setIdentity(KF.errorCovPost, Scalar::all(1));

	randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));

	bitwise_not(area, area);
	/*
	//Pra mexer com um canal só
	split(area, areaSplit);
	*/
	//Encontra os contornos
	findContours(area, contornos, hierarquia, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	

	//Se o contorno for grande o suficiente (?) acha o centro e armazena
	Point2f centro(0,0);

	state.at<float>(0, 0) = centro.x;
	state.at<float>(1, 0) = centro.y;
	state.copyTo(measurement);
	
	//for (int i = 0; i<contornos.size(); i++) {
		//if (contourArea(contornos[i]) >= tamMinimo) {
			float raio;
			minEnclosingCircle(contornos[0], centro, raio);

			Mat prediction = KF.predict();
			/*
			measurement.at<float>(0,0) = centro.x;
			measurement.at<float>(1, 0) = centro.y;
			*/


			circle(rastro, centro, 5, Scalar(0, 0, 255), -1);

			Point predictPoint(prediction.at<float>(0), prediction.at<float>(1));

			circle(rastro,predictPoint, 5, Scalar(255,0,0), -1);

			measurement += KF.measurementMatrix*state;
			Mat correct = KF.correct(measurement);
			Point correctPoint(correct.at<float>(0), correct.at<float>(1));

			circle(rastro, correctPoint, 5, Scalar(0, 255, 0), -1);

			state.at<float>(0, 0) = centro.x;
			state.at<float>(1, 0) = centro.y;

		//}		
	//}
}

int main(int argc, char** argv) {

	Mat media, variancia;
	Mat fundo, quadroAnterior;
	Mat rastro;
	VideoCapture vid(argv[1]);
	vector<Rastro> rastros;
	RotatedRect conjuntoDasVagas = RotatedRect(Point2f(550, 150), Size2f(200,70), 70);
	int cont = 0;

	

	MediaVariancia(1, argv[1], media, variancia);
	media.copyTo(rastro);

	Point2f vertices[4];
	conjuntoDasVagas.points(vertices);

	for (int i = 0; i < 4; i++)
		line(rastro, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));

	while (vid.isOpened()) {
		Mat quadro;
		if (!vid.read(quadro)) {
			break;
		}
		else {
			Mat D;
			rectangle(quadro, Point2f(250, 140), Point2f(400, 500), Scalar(0, 0, 0), -1);
			if (!quadroAnterior.empty()) {
				D = Diferenca(quadro, quadroAnterior, 1, atoi(argv[2]));
				//Se os quadros não forem exatamente iguais atualiza o rastro
				if (contaZeros(D)>0) {
					Mat estrut;
					estrut = Mat::ones(6, 6, CV_8UC1);
					dilate(D, D, estrut, Point(-1, -1), 5);
					Mat dBranca = Branco3Canais(D);
					erode(dBranca, dBranca, Mat::ones(3, 3, CV_8UC1), Point(-1, -1), 1);
					Kalman(dBranca, rastro, rastros);

				}
				//imshow("Dilatada", D);
				imshow("rastro", rastro);
				imshow("Video", quadro);
				//imshow("Anterior", quadroAnterior);

				waitKey(1);
			}

			quadro.copyTo(quadroAnterior);
		}
	}

	return 0;
}