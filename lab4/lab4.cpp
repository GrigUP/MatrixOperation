#include "stdafx.h"
#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <omp.h>

using namespace std;

const int totalNumThreads = 10;
const string pathMatrixA = "D:\\MatrixA.txt";
const string pathMatrixB = "D:\\MatrixB.txt";
const string pathMatrixC = "D:\\MatrixC.txt";
const string pathMatrixD = "D:\\MatrixD.txt";
const string pathResultMatrixEx1 = "D:\\restultMatrixEx1.txt";
const string pathResultMatrixEx2 = "D:\\restultMatrixEx2.txt";

class Matrix {
public:
	Matrix(vector<vector<double>> matrix) {
		this->matrix = matrix;
	}

	Matrix(string path) {
		readFile(path);
	}

	Matrix transposition() {
		vector<vector<double>> vectorResult(this->getColumn());
		for (int i = 0; i < vectorResult.size(); i++) {
			vector<double> subVector(this->getRow());
			vectorResult[i] = subVector;
		}

		int i;
		#pragma omp parallel for collapse(2)
		{
			for (i = 0; i < matrix.size(); i++) {
				vector<double> tempVector = matrix[i];
				for (int j = 0; j < tempVector.size(); j++) {
					double value = tempVector[j];
					vectorResult[j][i] = value;
				}
			}
		}

		Matrix result(vectorResult);
		return result;
	}

	Matrix inversion() {
		Matrix tempMatrix(matrix);
		double determinant = Matrix::determination(tempMatrix);
		if (determinant == 0) {
			cout << "Inverse matrix not exists!" << endl;
		}
		
		vector<vector<double>> vectorResult(tempMatrix.getRow());
		for (int i = 0; i < vectorResult.size(); i++) {
			vector<double> subVector(tempMatrix.getColumn());
			vectorResult[i] = subVector;
		}

		#pragma omp parallel for collapse(2) 
		{
			for (int i = 0; i < tempMatrix.getRow(); i++) {
				for (int j = 0; j < tempMatrix.getColumn(); j++)	{
					double value = Matrix::determination(Matrix::minor(tempMatrix, i, j));
					vectorResult[i][j] = value;
				}
			}
		}
		
		#pragma omp parallel for collapse(2) 
		{
			for (int i = 0; i < tempMatrix.getRow(); i++) {
				for (int j = 0; j < tempMatrix.getColumn(); j++) {
						vectorResult[i][j] = pow(-1, i + 1 + j + 1) * vectorResult[i][j];
				}
			}
		}

		Matrix result(vectorResult);
		result = result.transposition();
		
		#pragma omp parallel for collapse(2) 
		{
			for (int i = 0; i < tempMatrix.getRow(); i++) {
				for (int j = 0; j < tempMatrix.getColumn(); j++) {
					double value = result.getElement(i, j);
					result.setElement(i, j, value / determinant);
				}
			}
		}
		return result;

	}

	static Matrix multiplication(Matrix a, Matrix b) {
		vector<vector<double>> vectorResult(a.getRow());
		for (int i = 0; i < vectorResult.size(); i++) {
			vector<double> subVector(b.getColumn());
			vectorResult[i] = subVector;
		}

		#pragma omp parallel for collapse(3) 
		{
			for (int i = 0; i < a.getRow(); i++) {
				for (int j = 0; j < a.getRow(); j++) {
					double sum = 0;
					for (int k = 0; k < a.getColumn(); k++) {
						sum += a.getElement(i, k) * b.getElement(k, j);
					}
					vectorResult[i][j] = sum;
				}
			}
		}

		Matrix result(vectorResult);

		return result;
	}

	static double determination(Matrix a) {
		int matrixRow = a.getRow();
		int matrixColumn = a.getColumn();
		if (matrixRow != matrixColumn) {
			cout << "Error determination!" << endl;
			return NULL;
		}
		if (matrixRow == 2 && matrixColumn == 2)
			return (a.getElement(0, 0)*a.getElement(1, 1) - (a.getElement(1, 0)*a.getElement(0, 1)));

		int i;
		double sum = 0;

		#pragma omp parallel for reduction (+:sum)
		{
			for (i = 0; i < a.getColumn(); i++) {
				sum += pow(-1, 1 + i + 1) * a.getElement(0, i) * determination(minor(a, 0, i));
			}
		}

		return sum;
	}

	static Matrix addition(Matrix a, Matrix b) {
		vector<vector<double>> vectorResult(a.getRow());
		for (int i = 0; i < vectorResult.size(); i++) {
			vector<double> subVector(b.getColumn());
			vectorResult[i] = subVector;
		}

		#pragma omp parallel for collapse(2) 
		{
			for (int i = 0; i < a.getRow(); i++) {
				for (int j = 0; j < a.getColumn(); j++) {
					vectorResult[i][j] = a.getElement(i, j) + b.getElement(i, j);
				}
			}
		}

		Matrix result(vectorResult);

		return result;
	}

	static Matrix substraction(Matrix a, Matrix b) {
		vector<vector<double>> vectorResult(a.getRow());
		for (int i = 0; i < vectorResult.size(); i++) {
			vector<double> subVector(b.getColumn());
			vectorResult[i] = subVector;
		}

		#pragma omp parallel for collapse(2) 
		{
			for (int i = 0; i < a.getRow(); i++) {
				for (int j = 0; j < a.getColumn(); j++) {
					vectorResult[i][j] = a.getElement(i, j) - b.getElement(i, j);
				}
			}
		}

		Matrix result(vectorResult);

		return result;
	}

	static Matrix combine(Matrix a, Matrix b) {
		vector<vector<double>> vectorResult(a.getRow());
		for (int i = 0; i < vectorResult.size(); i++) {
			vector<double> subVector(b.getColumn()+a.getColumn());
			vectorResult[i] = subVector;
		}

		#pragma omp parallel for collapse(2) 
		{
			for (int i = 0; i < a.getRow(); i++) {
				for (int j = 0; j < a.getColumn(); j++) {
					vectorResult[i][j] = a.getElement(i, j);
				}
			}
		}

		#pragma omp parallel for collapse(2) 
		{
			for (int i = 0; i < b.getRow(); i++) {
				for (int j = 0; j < b.getColumn(); j++) {
					vectorResult[i][a.getColumn() + j] = b.getElement(i, j);
				}
			}
		}

		Matrix result(vectorResult);

		return result;
	}

	static Matrix modGauss(Matrix *slau) {
		vector<vector<double>> vectorResult(slau->getRow());
		for (int i = 0; i < vectorResult.size(); i++) {
			vector<double> subVector(1);
			vectorResult[i] = subVector;
		}

		// прямой ход
		getTriangleNormForm(slau);
		
		// обратный ход
		getTriangleInverseForm(slau);

		// определение корней
		for (int i = 0; i < slau->getRow(); i++) {
			vectorResult[i][0] = slau->getElement(i, slau->getColumn() - 1) / slau->getElement(i, i);
		}

		Matrix result(vectorResult);

		return result;
	}
	
	void printMatrix() {
		cout << "############################################" << endl;
		for (auto i = matrix.begin(); i < matrix.end(); i++) {
			for (const auto &j : *i) {
				cout.setf(ios::left);
				cout.width(10);
				cout << j << " ";
			}
			cout << endl;
		}

		cout << "############################################" << endl;
	}

	void writeFile(string path) {
		ofstream file(path, ios_base::out);

		if (!file.is_open()) cout << "file don't open" << endl;

		for (int i = 0; i < this->getRow(); i++) {
			for (int j = 0; j < this->getColumn(); j++) {
				file.setf(ios::left);
				file.width(10);
				file << matrix[i][j];
			}
			file << endl;
		}

		file.close();
	}


private:
	string pathMatrixCoefficientAnswer;
	int column = 0;
	int row = 0;
	vector<vector<double>> matrix;

	vector<double> getCoefficientFromString(string str) {
		vector<double> tempVector;
		int start_value = 0;
		for (int i = 0; i < str.length(); i++) {
			if (str[i] == ' ') {
				tempVector.push_back(stod(str.substr(start_value, i - start_value)));
				start_value = i + 1; 
			}
		}
		tempVector.push_back(stod(str.substr(start_value, str.length())));
		return tempVector;
	}

	void readFile(string path) {
		ifstream file(path, ios_base::in);

		if (!file.is_open()) cout << "file don't open" << endl;


		while (!file.eof()) {
			string str;
			getline(file, str);
			matrix.push_back(getCoefficientFromString(str));
			row++;
		}

		file.close();
	}

	static Matrix minor(Matrix a, int row, int column) {
		vector<vector<double>> vectorResult(a.getRow() - 1);
		for (int i = 0; i < vectorResult.size(); i++) {
			vector<double> subVector(a.getColumn() - 1);
			vectorResult[i] = subVector;
		}

		#pragma omp parallel for collapse(2)
		{
			for (int i = 0, iA = 0; i < a.getRow(); i++, iA++) {
				if (i == row) {
					iA--;
					continue;
				}
				for (int j = 0, jA = 0; j < a.getColumn(); j++, jA++) {
					if (j == column) {
						jA--;
						continue;
					}

					vectorResult[iA][jA] = a.getElement(i, j);
				}
			}
		}

		Matrix result(vectorResult);
		return result;
	}

	static void getTriangleNormForm(Matrix *slau) {
		
		#pragma omp parallel for collapse(3) 
		{
		changeRow(slau, 0);
			for (int j = 0; j < slau->getColumn(); j++) {
				for (int k = j + 1; k < slau->getRow(); k++) {
					if (slau->getElement(k, j) != 0) {
						double coeff = slau->getElement(j, j) / slau->getElement(k, j);
						for (int f = j; f < slau->getColumn(); f++) {
							slau->setElement(k, f, slau->getElement(j, f) - slau->getElement(k, f)*coeff);
						}
					}
				}
			}
		}
	}

	static void getTriangleInverseForm(Matrix *slau) {

		#pragma omp parallel for collapse(3) 
		{
			for (int i = 1; i < slau->getColumn() - 1; i++) {
				for (int j = 0; j < i; j++) {
					if (slau->getElement(j, i) != 0) {
						double coeff = slau->getElement(i, i) / slau->getElement(j, i);
						for (int k = 0; k < slau->getColumn(); k++) {
							slau->setElement(j, k, slau->getElement(j, k)* coeff - slau->getElement(i, k));
						}
					}
				}
			}
		}
	}

	static void changeRow(Matrix *slau, int column) {
		int i = slau->getMaxRowIndex(column);
		vector<double> maxCoefficientRow = slau->getVector()[i];
		vector<double> tempCoeff = slau->getVector()[0];
		slau->setVector(maxCoefficientRow, 0);
		slau->setVector(tempCoeff, i);
	}

	double getMaxRowIndex(int column) {
		double max = LONG_MIN;
		int index = 0;
		for (int i = 0; i < this->getRow(); i++) {
			if (abs(matrix[i][column]) > max) {
				max = abs(matrix[i][column]);
				index = i;
			}
		}

		return index;
	}

	int getColumn() {
		return matrix[0].size();
	}

	int getRow() {
		return matrix.size();
	}

	double getElement(int i, int j) {
		return matrix[i][j];
	}

	void setElement(int i, int j, double value) {
		matrix[i][j] = value;
	}

	vector<vector<double>> getVector() {
		return matrix;
	}

	void setVector(vector<double> vector, int index) {
		matrix[index] = vector;
	}

};

int main()
{
	omp_set_num_threads(totalNumThreads);
	
	Matrix matrixA(pathMatrixA);
	Matrix matrixB(pathMatrixB);
	Matrix matrixC(pathMatrixC);
	Matrix matrixD(pathMatrixD);

	Matrix A("D:\\A.txt");
	Matrix B("D:\\B.txt");
	B = B.transposition();

	/*Matrix resultEx1 = Matrix::substraction(Matrix::substraction(matrixC.transposition(), matrixD.inversion()), Matrix::multiplication(matrixA, matrixB));
	cout << "Result = " << endl;
	resultEx1.printMatrix();
	resultEx1.writeFile(pathResultMatrixEx1);*/
	
	Matrix resultEx2 = Matrix::modGauss(&Matrix::combine(A, B));
	cout << "Result = " << endl;
	resultEx2.printMatrix();
	resultEx2.writeFile(pathResultMatrixEx2);

	system("pause");
	return 0;
}
