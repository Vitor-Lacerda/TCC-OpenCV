#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>
//#include "Comparador.h"
#include "VerificadorVagas.h"
#include "Structs.h"

using namespace cv;
using namespace std;

struct MouseParams {
	vector<Vaga> vetor;
	Point primeiro;
	Point segundo;
	vector<Mat> imagens;
	//Comparador comp;
	VerificadorVagas verifVagas;
	
};


Vaga novaVaga(Rect r) {
	Vaga n;
	n.limites = r;
	n.ocupada = false;
	
	return n;

}

Mat desenhaVagas;


void myMouseCallback(int event, int x, int y, int, void* param) {

	MouseParams* v = (MouseParams*)param;

	if (event == EVENT_LBUTTONDOWN) {
		v->primeiro = Point(x,y);
	}
	if (event == EVENT_LBUTTONUP) {
		v->segundo = Point(x,y);

		int w = abs(v->segundo.x - v->primeiro.x);
		int h = abs(v->segundo.y - v->primeiro.y);

		Rect r = Rect(v->primeiro.x, v->primeiro.y, w, h);
		v->vetor.push_back(novaVaga(r));

		rectangle(desenhaVagas, r, Scalar(255, 0, 0), 1, 8, 0);
		imshow("Principal", desenhaVagas);
	}
	if (event == EVENT_RBUTTONDOWN) {
		//v->comp.Iniciar(v->imagens, v->vetor);
		v->verifVagas.Iniciar(v->imagens, v->vetor);
	}

	

}

int main(int argc, char** argv[]) {

	MouseParams mps;
	
	string endereco = "../ImagensTeste/Animacao/Animacao";

	for (int i = 0; i < 84; i++) {
		ostringstream oss;
		oss << endereco << i << ".png";
		Mat img = imread(oss.str(), CV_32FC3);
		mps.imagens.push_back(img);
	}
	mps.imagens[0].copyTo(desenhaVagas);
	namedWindow("Principal", 1);
	imshow("Principal", mps.imagens[0]);
	setMouseCallback("Principal", myMouseCallback, (void*)&mps);

	waitKey(0);


	return 0;
}