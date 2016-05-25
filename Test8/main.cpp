﻿/**
* 論文Image-based procedural modeling of facadesに基づいて、
* Facadeを分割する。
* 5/24に第一回の報告をしたが、いろいろ難しいことが分かった。
* まず、MIではうまくsymmetryをキャプチャできないケースがある。
*
* @author Gen Nishida
* @version 1.0
*/

#include <algorithm>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <opencv2/opencv.hpp>
#include "CVUtils.h"
#include "Utils.h"

using namespace std;

class Subdivision {
public:
	static enum { TYPE_LEFT = 0, TYPE_RIGHT, TYPE_TOP, TYPE_BOTTOM };

public:
	int dir;
	bool dual;
	int dist;

public:
	Subdivision() : dir(0), dual(false), dist(0) {}
	Subdivision(int dir, int dual, int dist) : dir(dir), dual(dual), dist(dist) {}
};

/**
 * Facade画像と合わせて、S_max(y)とh_max(y)を画像として保存する。
 * 論文 Fig 5に相当する画像。
 *
 * @param img		Facade画像
 * @param S_max		S_max
 * @param h_max		h_max
 * @param filename	output file name
 */
void outputFacadeStructureV(const cv::Mat& img, const cv::Mat_<float>& S_max, const cv::Mat_<float>& h_max, const vector<int>& y_set, const string& filename) {
	float max_S = cvutils::max(S_max);
	float min_S = cvutils::min(S_max);
	float max_h = cvutils::max(h_max);
	float min_h = cvutils::min(h_max);

	int graphSize = img.rows * 0.25;
	int margin = graphSize * 0.2;
	cv::Mat result(img.rows, img.cols + graphSize + max_h + margin * 3, CV_8UC3, cv::Scalar(255, 255, 255));

	// copy img to result
	cv::Mat roi(result, cv::Rect(0, 0, img.cols, img.rows));
	img.copyTo(roi);

	// draw S_max
	for (int r = 0; r < img.rows - 1; ++r) {
		int x1 = img.cols + margin + (S_max(r, 0) - min_S) / (max_S - min_S) * graphSize;
		int x2 = img.cols + margin + (S_max(r + 1, 0) - min_S) / (max_S - min_S) * graphSize;

		cv::line(result, cv::Point(x1, r), cv::Point(x2, r + 1), cv::Scalar(0, 0, 0), 1, cv::LINE_8);
	}

	// draw h_max
	for (int r = 0; r < img.rows - 1; ++r) {
		int x1 = img.cols + graphSize + margin * 2 + h_max(r, 0);
		int x2 = img.cols + graphSize + margin * 2 + h_max(r + 1, 0);

		cv::line(result, cv::Point(x1, r), cv::Point(x2, r + 1), cv::Scalar(0, 0, 0), 1, cv::LINE_8);
	}

	// draw the horizontal split lines
	for (int i = 0; i < y_set.size(); ++i) {
		cv::line(result, cv::Point(0, y_set[i]), cv::Point(img.cols - 1, y_set[i]), cv::Scalar(0, 0, 255), 1);
	}

	cv::imwrite(filename, result);
}

void outputFacadeStructureV(const cv::Mat& img, const cv::Mat_<float>& S_max, const cv::Mat_<float>& h_max, const string& filename) {
	vector<int> y_set;
	outputFacadeStructureV(img, S_max, h_max, y_set, filename);
}

void outputFacadeStructureH(const cv::Mat& img, const cv::Mat_<float>& S_max, const cv::Mat_<float>& w_max, const vector<int>& x_set, const string& filename) {
	float max_S = cvutils::max(S_max);
	float min_S = cvutils::min(S_max);
	float max_w = cvutils::max(w_max);
	float min_w = cvutils::min(w_max);

	int graphSize = max(80.0, img.rows * 0.25);
	int margin = graphSize * 0.2;
	cv::Mat result(img.rows + graphSize + max_w + margin * 3, img.cols, CV_8UC3, cv::Scalar(255, 255, 255));

	// copy img to result
	cv::Mat roi(result, cv::Rect(0, 0, img.cols, img.rows));
	img.copyTo(roi);

	// draw S_max
	for (int c = 0; c < img.cols - 1; ++c) {
		int y1 = img.rows + margin + (S_max(0, c) - min_S) / (max_S - min_S) * graphSize;
		int y2 = img.rows + margin + (S_max(0, c + 1) - min_S) / (max_S - min_S) * graphSize;

		cv::line(result, cv::Point(c, y1), cv::Point(c + 1, y2), cv::Scalar(0, 0, 0), 1, cv::LINE_8);
	}

	// draw w_max
	for (int c = 0; c < img.cols - 1; ++c) {
		int y1 = img.rows + graphSize + margin * 2 + w_max(0, c);
		int y2 = img.rows + graphSize + margin * 2 + w_max(0, c + 1);

		cv::line(result, cv::Point(c, y1), cv::Point(c + 1, y2), cv::Scalar(0, 0, 0), 1, cv::LINE_8);
	}

	// draw the vertical split lines
	for (int i = 0; i < x_set.size(); ++i) {
		cv::line(result, cv::Point(x_set[i], 0), cv::Point(x_set[i], img.rows - 1), cv::Scalar(0, 0, 255), 1);
	}

	cv::imwrite(filename, result);
}

void outputFacadeStructureH(const cv::Mat& img, const cv::Mat_<float>& S_max, const cv::Mat_<float>& w_max, const string& filename) {
	vector<int> x_set;
	outputFacadeStructureH(img, S_max, w_max, x_set, filename);
}

void outputFacadeStructure(const cv::Mat& img, const cv::Mat_<float>& SV_max, const cv::Mat_<float>& h_max, const cv::Mat_<float>& SH_max, const cv::Mat_<float>& w_max, const vector<int>& y_set, const vector<int>& x_set, const string& filename) {
	float max_SV = cvutils::max(SV_max);
	float min_SV = cvutils::min(SV_max);
	float max_h = cvutils::max(h_max);
	float min_h = cvutils::min(h_max);
	float max_SH = cvutils::max(SH_max);
	float min_SH = cvutils::min(SH_max);
	float max_w = cvutils::max(w_max);
	float min_w = cvutils::min(w_max);

	int graphSize = max(80.0, img.rows * 0.25);
	int margin = graphSize * 0.2;
	cv::Mat result(img.rows + graphSize + max_w + margin * 3, img.cols + graphSize + max_h + margin * 3, CV_8UC3, cv::Scalar(255, 255, 255));

	// copy img to result
	cv::Mat roi(result, cv::Rect(0, 0, img.cols, img.rows));
	img.copyTo(roi);

	// draw SV_max
	for (int r = 0; r < img.rows - 1; ++r) {
		int x1 = img.cols + margin + (SV_max(r, 0) - min_SV) / (max_SV - min_SV) * graphSize;
		int x2 = img.cols + margin + (SV_max(r + 1, 0) - min_SV) / (max_SV - min_SV) * graphSize;

		cv::line(result, cv::Point(x1, r), cv::Point(x2, r + 1), cv::Scalar(0, 0, 0), 1, cv::LINE_8);
	}

	// draw h_max
	for (int r = 0; r < img.rows - 1; ++r) {
		int x1 = img.cols + graphSize + margin * 2 + h_max(r, 0);
		int x2 = img.cols + graphSize + margin * 2 + h_max(r + 1, 0);

		cv::line(result, cv::Point(x1, r), cv::Point(x2, r + 1), cv::Scalar(0, 0, 0), 1, cv::LINE_8);
	}

	// draw SH_max
	for (int c = 0; c < img.cols - 1; ++c) {
		int y1 = img.rows + margin + (SH_max(0, c) - min_SH) / (max_SH - min_SH) * graphSize;
		int y2 = img.rows + margin + (SH_max(0, c + 1) - min_SH) / (max_SH - min_SH) * graphSize;

		cv::line(result, cv::Point(c, y1), cv::Point(c + 1, y2), cv::Scalar(0, 0, 0), 1, cv::LINE_8);
	}

	// draw w_max
	for (int c = 0; c < img.cols - 1; ++c) {
		int y1 = img.rows + graphSize + margin * 2 + w_max(0, c);
		int y2 = img.rows + graphSize + margin * 2 + w_max(0, c + 1);

		cv::line(result, cv::Point(c, y1), cv::Point(c + 1, y2), cv::Scalar(0, 0, 0), 1, cv::LINE_8);
	}

	// draw the horizontal split lines
	for (int i = 0; i < y_set.size(); ++i) {
		cv::line(result, cv::Point(0, y_set[i]), cv::Point(img.cols - 1, y_set[i]), cv::Scalar(0, 0, 255), 1);
	}

	// draw the vertical split lines
	for (int i = 0; i < x_set.size(); ++i) {
		cv::line(result, cv::Point(x_set[i], 0), cv::Point(x_set[i], img.rows - 1), cv::Scalar(0, 0, 255), 1);
	}

	cv::imwrite(filename, result);
}

void outputFacadeStructure(const cv::Mat& img, const cv::Mat_<float>& SV_max, const cv::Mat_<float>& h_max, const cv::Mat_<float>& SH_max, const cv::Mat_<float>& w_max, const string& filename) {
	vector<int> x_set;
	vector<int> y_set;
	outputFacadeStructure(img, SV_max, h_max, SH_max, w_max, y_set, x_set, filename);
}

void outputFacadeStructure(const cv::Mat& img, const vector<int>& y_set, const vector<int>& x_set, const string& filename) {
	cv::Mat result = img.clone();

	for (int i = 0; i < y_set.size(); ++i) {
		cv::line(result, cv::Point(0, y_set[i]), cv::Point(img.cols, y_set[i]), cv::Scalar(0, 0, 255), 3);
	}
	for (int i = 0; i < x_set.size(); ++i) {
		cv::line(result, cv::Point(x_set[i], 0), cv::Point(x_set[i], img.rows), cv::Scalar(0, 0, 255), 3);
	}
	cv::imwrite(filename, result);
}

void drawSubdivisionOnTile(cv::Mat& tile, const vector<Subdivision>& subdivisions, const cv::Scalar& color, int thickness) {
	int x1 = 0;
	int x2 = tile.cols - 1;
	int y1 = 0;
	int y2 = tile.rows - 1;

	for (int i = 0; i < subdivisions.size(); ++i) {
		if (subdivisions[i].dir == Subdivision::TYPE_LEFT) {
			x1 += subdivisions[i].dist;
			cv::line(tile, cv::Point(x1, y1), cv::Point(x1, y2), color, thickness);
			if (subdivisions[i].dual) {
				x2 -= subdivisions[i].dist;
				cv::line(tile, cv::Point(x2, y1), cv::Point(x2, y2), color, thickness);
			}
		}
		else if (subdivisions[i].dir == Subdivision::TYPE_RIGHT) {
			x2 -= subdivisions[i].dist;
			cv::line(tile, cv::Point(x2, y1), cv::Point(x2, y2), color, thickness);
			if (subdivisions[i].dual) {
				x1 -= subdivisions[i].dist;
				cv::line(tile, cv::Point(x1, y1), cv::Point(x1, y2), color, thickness);
			}
		}
		else if (subdivisions[i].dir == Subdivision::TYPE_TOP) {
			y1 += subdivisions[i].dist;
			cv::line(tile, cv::Point(x1, y1), cv::Point(x2, y1), color, thickness);
			if (subdivisions[i].dual) {
				y2 -= subdivisions[i].dist;
				cv::line(tile, cv::Point(x1, y2), cv::Point(x2, y2), color, thickness);
			}
		}
		else if (subdivisions[i].dir == Subdivision::TYPE_BOTTOM) {
			y2 -= subdivisions[i].dist;
			cv::line(tile, cv::Point(x1, y2), cv::Point(x2, y2), color, thickness);
			if (subdivisions[i].dual) {
				y1 -= subdivisions[i].dist;
				cv::line(tile, cv::Point(x1, y1), cv::Point(x2, y1), color, thickness);
			}
		}
	}
}

void outputFacadeAndTileStructure(const cv::Mat& img, const vector<vector<int>>& y_set, const vector<vector<int>>& x_set, const vector<vector<vector<Subdivision>>>& subdivisions, const string& filename) {
	cv::Mat img2 = img.clone();

	// visualize the subdivision of tiles
	for (int i = 0; i < y_set.size(); ++i) {
		for (int j = 0; j < y_set[i].size() - 1; ++j) {
			int y1 = y_set[i][j];
			int y2 = y_set[i][j + 1];

			for (int k = 0; k < x_set.size(); ++k) {
				for (int l = 0; l < x_set[k].size() - 1; ++l) {
					int x1 = x_set[k][l];
					int x2 = x_set[k][l + 1];

					int u1 = x1;
					int u2 = x2;
					int v1 = y1;
					int v2 = y2;

					cv::Mat tile(img2, cv::Rect(x1, y1, x2 - x1 - 1, y2 - y1 - 1));
					drawSubdivisionOnTile(tile, subdivisions[i][k], cv::Scalar(255, 0, 0), 3);
				}
			}
		}
	}

	// visualize the subdivision of facade
	for (int i = 0; i < y_set.size(); ++i) {
		for (int j = 0; j < y_set[i].size(); ++j) {
			cv::line(img2, cv::Point(0, y_set[i][j]), cv::Point(img2.cols - 1, y_set[i][j]), cv::Scalar(0, 0, 255), 3);
		}
	}
	for (int i = 0; i < x_set.size(); ++i) {
		for (int j = 0; j < x_set[i].size(); ++j) {
			cv::line(img2, cv::Point(x_set[i][j], 0), cv::Point(x_set[i][j], img2.rows - 1), cv::Scalar(0, 0, 255), 3);
		}
	}

	cv::imwrite(filename, img2);
}

/**
* IFデータに基づいて画像を生成する。
*
* @param	IF		IrreducibleFacade
* @param	imgIF	Image of IF
*/
void createIFImage(const cv::Mat& IF, cv::Mat& imgIF) {
	imgIF = cv::Mat(IF.rows, IF.cols, CV_8UC3);

	for (int r = 0; r < IF.rows; ++r) {
		for (int c = 0; c < IF.cols; ++c) {
			int blue = IF.at<cv::Vec4f>(r, c)[0] / IF.at<cv::Vec4f>(r, c)[3];
			int green = IF.at<cv::Vec4f>(r, c)[1] / IF.at<cv::Vec4f>(r, c)[3];
			int red = IF.at<cv::Vec4f>(r, c)[2] / IF.at<cv::Vec4f>(r, c)[3];
			imgIF.at<cv::Vec3b>(r, c) = cv::Vec3b(blue, green, red);
		}
	}
}

void outputIF(const cv::Mat& IF, const string& filename) {
	cv::Mat imgIF;
	createIFImage(IF, imgIF);

	cv::imwrite(filename, imgIF);
}

/**
 * 2つの領域の類似度を返却する。
 *
 * @param R1		領域1 (1-channel image)
 * @param R2		領域2 (1-channel image)
 * @return			類似度
 */
float MI(const cv::Mat& R1, const cv::Mat& R2) {
	return expf(-cvutils::msd(R1, R2) * 0.001f);

	/*
	cv::Mat_<float> Pab(256, 256, 0.0f);
	cv::Mat_<float> Pa(256, 1, 0.0f);
	cv::Mat_<float> Pb(256, 1, 0.0f);

	// create a histogram of intensities
	for (int r = 0; r < R1.rows; ++r) {
		for (int c = 0; c < R1.cols; ++c) {
			int a = R1.at<unsigned char>(r, c);
			int b = R2.at<unsigned char>(r, c);

			Pab(a, b)++;
			Pa(a, 0)++;
			Pb(b, 0)++;
		}
	}

	// normalize the historgram
	cv::Mat Pab_max;
	cv::Mat Pa_max;
	cv::Mat Pb_max;
	cv::reduce(Pab, Pab_max, 0, CV_REDUCE_SUM);
	cv::reduce(Pab_max, Pab_max, 1, CV_REDUCE_SUM);
	cv::reduce(Pa, Pa_max, 0, CV_REDUCE_SUM);
	cv::reduce(Pb, Pb_max, 0, CV_REDUCE_SUM);
	Pab /= Pab_max.at<float>(0, 0);
	Pa /= Pa_max.at<float>(0, 0);
	Pb /= Pb_max.at<float>(0, 0);

	float result = 0.0f;
	for (int a = 0; a < 256; ++a) {
		for (int b = 0; b < 256; ++b) {
			float v = Pab(a, b);
			if (v == 0) {
				v = 0.001f * 0.001f;
			}
			float v1 = Pa(a, 0);
			if (v1 == 0) v1 = 0.001f;
			float v2 = Pb(b, 0);
			if (v2 == 0) v2 = 0.001f;

			float hoge = Pab(a, b) * log(v / v1 / v2);
			result += Pab(a, b) * log(v / v1 / v2);
		}
	}

	return result;
	*/
}

/**
* Irreducible Facadeの、[y, y+h]を[y-h, y]へ移動してshrinkさせる。
*
* @param	IF		Irreducible Facade
* @param	y		Splitの位置
* @param	h		Overlapさせる高さ
*/
void vshrinkIF(cv::Mat& IF, int y, int h) {
	h = std::min(h, IF.rows - y);

	cv::Mat IF_bak = IF.clone();
	IF = cv::Mat(IF.rows - h, IF.cols, CV_32FC4);

	// overlapping部分より上側をコピー
	for (int r = 0; r < y; ++r) {
		for (int c = 0; c < IF.cols; ++c) {
			IF.at<cv::Vec4f>(r, c) = IF_bak.at<cv::Vec4f>(r, c);
		}
	}

	// overlapping部分をコピー
	for (int r = y - h; r < y; ++r) {
		for (int c = 0; c < IF.cols; ++c) {
			IF.at<cv::Vec4f>(r, c) = IF.at<cv::Vec4f>(r, c) + IF_bak.at<cv::Vec4f>(r + h, c);
		}
	}

	// overlapping部分より下側をコピー
	for (int r = y; r < IF.rows; ++r) {
		for (int c = 0; c < IF.cols; ++c) {
			IF.at<cv::Vec4f>(r, c) = IF_bak.at<cv::Vec4f>(r + h, c);
		}
	}
}

/**
 * Irredicible facadeを垂直方向に、split位置y_setに従ってshrinkさせる。
 * ただし、y_setは昇順に並んでいるものとする。
 */
void vshrinkIF(cv::Mat& IF, const vector<vector<int>>& y_set, const cv::Mat_<float>& h_max) {
	for (int i = y_set.size() - 1; i >= 0; --i) {
		for (int j = y_set[i].size() - 2; j >= 1; --j) {
			vshrinkIF(IF, y_set[i][j], h_max(y_set[i][j], 0));
		}
	}
}

/**
* Irreducible Facadeの、[x, x+w]を[x-w, x]へ移動してshrinkさせる。
*
* @param	IF		Irreducible Facade
* @param	x		Splitの位置
* @param	w		Overlapさせる幅
*/
void hshrinkIF(cv::Mat& IF, int x, int w) {
	w = std::min(w, IF.cols - x);

	cv::Mat IF_bak = IF.clone();
	IF = cv::Mat(IF.rows, IF.cols - w, CV_32FC4);

	// overlapping部分より左側をコピー
	for (int c = 0; c < x; ++c) {
		for (int r = 0; r < IF.rows; ++r) {
			IF.at<cv::Vec4f>(r, c) = IF_bak.at<cv::Vec4f>(r, c);
		}
	}

	// overlapping部分をコピー
	for (int c = x - w; c < x; ++c) {
		for (int r = 0; r < IF.rows; ++r) {
			IF.at<cv::Vec4f>(r, c) = IF.at<cv::Vec4f>(r, c) + IF_bak.at<cv::Vec4f>(r, c + w);
		}
	}

	// overlapping部分より右側をコピー
	for (int c = x; c < IF.cols; ++c) {
		for (int r = 0; r < IF.rows; ++r) {
			IF.at<cv::Vec4f>(r, c) = IF_bak.at<cv::Vec4f>(r, c + w);
		}
	}
}

/**
 * Irreducible facadeを、splitラインの位置x_setに基づいてshrinkさせる。
 * ただし、x_setは、昇順に並んでいるものとする。
 */
void hshrinkIF(cv::Mat& IF, const vector<vector<int>>& x_set, const cv::Mat_<float>& w_max) {
	for (int i = x_set.size() - 1; i >= 0; --i) {
		for (int j = x_set[i].size() - 2; j >= 1; --j) {
			hshrinkIF(IF, x_set[i][j], w_max(0, x_set[i][j]));
		}
	}
}

/**
* Facade画像のS_max(y)、h_max(y)を計算する。
*
* @param img		Facade画像 (1-channel image)
* @param SV_max		S_max(y)
* @param h_max		h_max(y)
* @param h_range	range of h
*/
void computeSV(const cv::Mat& img, cv::Mat_<float>& SV_max, cv::Mat_<float>& h_max, const pair<int, int>& h_range) {
	SV_max = cv::Mat_<float>(img.rows, 1, 0.0f);
	h_max = cv::Mat_<float>(img.rows, 1, 0.0f);

	ifstream in_SV_max("SV_max.txt");
	ifstream in_h("h_max.txt");
	if (in_SV_max.good() && in_h.good()) {
		in_SV_max.close();
		in_h.close();
		SV_max = cvutils::read("SV_max.txt");
		h_max = cvutils::read("h_max.txt");
	}
	else {
		printf("computing");
		for (int r = 0; r < img.rows; ++r) {
			printf("\rcomputing r = %d/%d  ", r, img.rows);

			cv::Mat_<float> SV(img.rows, 1, 0.0f);

			for (int h = h_range.first; h <= h_range.second; ++h) {
				if (r - h < 0 || r + h >= img.rows) continue;

				cv::Mat R1 = img(cv::Rect(0, r, img.cols, h));
				cv::Mat R2 = img(cv::Rect(0, r - h, img.cols, h));
				SV(h, 0) = MI(R1, R2);
				
				if (SV(h, 0) > SV_max(r, 0)) {
					SV_max(r, 0) = SV(h, 0);
					h_max(r, 0) = h;
				}
			}
		}
		printf("\n");

		// output SV_max(x) and h_max(x)
		cvutils::write("SV_max.txt", SV_max);
		cvutils::write("h_max.txt", h_max);
	}


	////////////////////////////////////////////////////////////////////////
	// DEBUG
#if 0
	{
		cv::Mat_<float> histograms(grayImg.rows, 1, 0.0f);
		int y0 = 147;
		for (int r = 0; r < grayImg.rows; ++r) {
			if (abs(r - y0) < 10) continue;

			if (r < y0) {
				int h = y0 - r;
				if (y0 + h >= grayImg.rows) continue;
				cv::Mat R1 = grayImg(cv::Rect(0, r, grayImg.cols, h));
				cv::Mat R2 = grayImg(cv::Rect(0, r + h, grayImg.cols, h));
				histograms(r, 0) += MI(R1, R2);
			}
			else {
				int h = r - y0;
				if (y0 - h < 0) continue;
				cv::Mat R1 = grayImg(cv::Rect(0, y0 - h, grayImg.cols, h));
				cv::Mat R2 = grayImg(cv::Rect(0, y0, grayImg.cols, h));
				histograms(r, 0) += MI(R1, R2);
			}
		}

		cvutils::outputImageWithVerticalGraph(img, histograms, "temp.png");

		float max_value = 0.0f;
		int max_index = -1;
		for (int i = 0; i < histograms.rows; ++i) {
			if (histograms(i, 0) > max_value) {
				max_value = histograms(i, 0);
				max_index = i;
			}
		}

		if (max_index < y0) {
			int h = y0 - max_index;
			cv::Mat R1 = grayImg(cv::Rect(0, max_index, grayImg.cols, h));
			cv::Mat R2 = grayImg(cv::Rect(0, max_index + h, grayImg.cols, h));
			cv::imwrite("R1.png", R1);
			cv::imwrite("R2.png", R2);
			cout << "MSD: " << cvutils::msd(R1, R2) << endl;
		}
		else {
			int h = max_index - y0;
			cv::Mat R1 = grayImg(cv::Rect(0, y0 - h, grayImg.cols, h));
			cv::Mat R2 = grayImg(cv::Rect(0, y0, grayImg.cols, h));
			cv::imwrite("R1.png", R1);
			cv::imwrite("R2.png", R2);
			cout << "MSD: " << cvutils::msd(R1, R2) << endl;
		}
	}
#endif
	////////////////////////////////////////////////////////////////////////
}

/**
 * Facade画像のS_max(x)、w_max(x)を計算する。
 *
 * @param img		Facade画像 (1-channel image)
 * @param SH_max	S_max(x)
 * @param w_max		w_max(x)
 * @param w_range	range of w
 */
void computeSH(const cv::Mat& img, cv::Mat_<float>& SH_max, cv::Mat_<float>& w_max, const pair<int, int>& w_range) {
	SH_max = cv::Mat_<float>(1, img.cols, 0.0f);
	w_max = cv::Mat_<float>(1, img.cols, 0.0f);

	ifstream in_SH("SH_max.txt");
	ifstream in_w("w_max.txt");
	if (in_SH.good() && in_w.good()) {
		in_SH.close();
		in_w.close();
		SH_max = cvutils::read("SH_max.txt");
		w_max = cvutils::read("w_max.txt");
	}
	else {
		printf("computing");
		for (int c = 0; c < img.cols; ++c) {
			printf("\rcomputing c = %d/%d  ", c, img.cols);

			cv::Mat_<float> SH(1, img.cols, 0.0f);

			for (int w = w_range.first; w <= w_range.second; ++w) {
				if (c - w < 0 || c + w >= img.cols) continue;

				cv::Mat R1 = img(cv::Rect(c, 0, w, img.rows));
				cv::Mat R2 = img(cv::Rect(c - w, 0, w, img.rows));

				SH(0, w) = MI(R1, R2);

				if (SH(0, w) > SH_max(0, c)) {
					SH_max(0, c) = SH(0, w);
					w_max(0, c) = w;
				}
			}
		}
		printf("\n");

		// output SH_max(x) and w_max(x)
		cvutils::write("SH_max.txt", SH_max);
		cvutils::write("w_max.txt", w_max);
	}


	////////////////////////////////////////////////////////////////////////
	// DEBUG
#if 0
	{
		cv::Mat_<float> histograms(1, grayImg.cols, 0.0f);
		int x0 = 153;
		for (int c = 0; c < grayImg.cols; ++c) {
			if (abs(c - x0) < 10) continue;

			if (c < x0) {
				int w = x0 - c;
				if (x0 + w >= grayImg.cols) continue;
				cv::Mat R1 = grayImg(cv::Rect(c, 0, w, grayImg.rows - 1));
				cv::Mat R2 = grayImg(cv::Rect(c + w, 0, w, grayImg.rows - 1));
				histograms(0, c) += MI(R1, R2);
			}
			else {
				int w = c - x0;
				if (x0 - w < 0) continue;
				cv::Mat R1 = grayImg(cv::Rect(x0 - w, 0, w, grayImg.rows));
				cv::Mat R2 = grayImg(cv::Rect(x0, 0, w, grayImg.rows));
				histograms(0, c) += MI(R1, R2);
			}
		}

		cvutils::outputImageWithHorizontalGraph(img, histograms, "temp.png");

		float max_value = 0.0f;
		int max_index = -1;
		for (int i = 0; i < histograms.cols; ++i) {
			if (histograms(0, i) > max_value) {
				max_value = histograms(0, i);
				max_index = i;
			}
		}

		if (max_index < x0) {
			int w = x0 - max_index;
			cv::Mat R1 = grayImg(cv::Rect(max_index, 0, w, grayImg.rows - 1));
			cv::Mat R2 = grayImg(cv::Rect(max_index + w, 0, w, grayImg.rows - 1));
			cv::imwrite("R1.png", R1);
			cv::imwrite("R2.png", R2);
			cout << "MSD: " << cvutils::msd(R1, R2) << endl;
		}
		else {
			int w = max_index - x0;
			cv::Mat R1 = grayImg(cv::Rect(x0 - w, 0, w, grayImg.rows - 1));
			cv::Mat R2 = grayImg(cv::Rect(x0, 0, w, grayImg.rows - 1));
			cv::imwrite("R1.png", R1);
			cv::imwrite("R2.png", R2);
			cout << "MSD: " << cvutils::msd(R1, R2) << endl;
		}
	}
#endif
	////////////////////////////////////////////////////////////////////////
}

/**
 * imgから、Ver(y)とHor(x)を計算する。
 *
 * @param img		image (3-channel color image)
 * @param Ver		Ver(y)
 * @param Hor		Hor(x)
 */
void computeVerAndHor(const cv::Mat& img, cv::Mat_<float>& Ver, cv::Mat_<float>& Hor, float sigma) {
	cv::Mat grayImg;
	cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);

	float alpha = 0.9f;

	// compute hor(x,y) and ver(x,y) according to Equation (3)
	cv::Mat_<float> hor(grayImg.rows, grayImg.cols);
	cv::Mat_<float> ver(grayImg.rows, grayImg.cols);
	for (int r = 0; r < grayImg.rows; ++r) {
		for (int c = 0; c < grayImg.cols; ++c) {
			float dIdx;
			float dIdy;

			if (c == 0) {
				dIdx = grayImg.at<unsigned char>(r, c + 1) - grayImg.at<unsigned char>(r, c);
			}
			else if (c == grayImg.cols - 1) {
				dIdx = grayImg.at<unsigned char>(r, c) - grayImg.at<unsigned char>(r, c - 1);
			}
			else {
				dIdx = (grayImg.at<unsigned char>(r, c + 1) - grayImg.at<unsigned char>(r, c - 1)) / 2.0f;
			}

			if (r == 0) {
				dIdy = grayImg.at<unsigned char>(r + 1, c) - grayImg.at<unsigned char>(r, c);
			}
			else if (r == grayImg.rows - 1) {
				dIdy = grayImg.at<unsigned char>(r, c) - grayImg.at<unsigned char>(r - 1, c);
			}
			else {
				dIdy = (float)(grayImg.at<unsigned char>(r + 1, c) - grayImg.at<unsigned char>(r - 1, c)) / 2.0f;
			}

			hor(r, c) = std::max(dIdy * dIdy * (1.0f - alpha) - dIdx * dIdx * alpha, 0.0f);
			ver(r, c) = std::max(dIdx * dIdx * (1.0f - alpha) - dIdy * dIdy * alpha, 0.0f);
		}
	}

	// sum up the ver(x, y) horizontally and vertically, respectively
	cv::Mat ver_xtotal;
	cv::Mat ver_ytotal;
	cv::reduce(ver, ver_xtotal, 1, cv::REDUCE_SUM);
	cv::reduce(ver, ver_ytotal, 0, cv::REDUCE_SUM);

	// sum up the hor(x, y) horizontally and vertically, respectively
	cv::Mat hor_xtotal;
	cv::Mat hor_ytotal;
	cv::reduce(hor, hor_xtotal, 1, cv::REDUCE_SUM);
	cv::reduce(hor, hor_ytotal, 0, cv::REDUCE_SUM);

	// compute Ver(y) and Hor(x) according to Equation (4)
	Ver = cv::Mat_<float>(grayImg.rows, 1, 0.0f);
	Hor = cv::Mat_<float>(1, grayImg.cols, 0.0f);
	float beta = 0.1f;
	for (int r = 0; r < grayImg.rows; ++r) {
		for (int rr = 0; rr < grayImg.rows; ++rr) {
			Ver(r, 0) += (ver_xtotal.at<float>(rr, 0) - beta * hor_xtotal.at<float>(rr, 0)) * utils::gause(rr - r, sigma);
		}
	}
	for (int c = 0; c < grayImg.cols; ++c) {
		for (int cc = 0; cc < grayImg.cols; ++cc) {
			Hor(0, c) += (hor_ytotal.at<float>(0, cc) - beta * ver_ytotal.at<float>(0, cc)) * utils::gause(cc - c, sigma);
		}
	}
}

/**
 * tileを分割し、分割方向、分割タイプ、ボーダーからの距離を返却する。
 * 分割しない場合はfalseを返却する。
 */
bool subdivideTile(cv::Mat& tile, Subdivision& subdivide, int min_size) {
	if (tile.cols < min_size || tile.rows < min_size) return false;

	cv::imwrite("tile.png", tile);

	cv::Mat_<float> Ver;
	cv::Mat_<float> Hor;
	computeVerAndHor(tile, Ver, Hor, 5.0f);

	// visualize Ver(y) and Hor(x)
	cvutils::outputImageWithHorizontalAndVerticalGraph(tile, Ver, Hor, "tile2.png");


	// find the local minima of Ver(y) and Hor(x)
	vector<int> y_set;
	for (int r = min_size; r < Ver.rows - min_size; ++r) {
		if (Ver(r, 0) < Ver(r - 1, 0) && Ver(r, 0) < Ver(r + 1, 0)) {
			y_set.push_back(r);
		}
	}
	vector<int> x_set;
	for (int c = min_size; c < Hor.cols - min_size; ++c) {
		if (Hor(0, c) < Hor(0, c - 1) && Hor(0, c) < Hor(0, c + 1)) {
			x_set.push_back(c);
		}
	}

	if (x_set.size() == 0 && y_set.size() == 0) return false;

	// find the split closest to the boundary
	subdivide.dist = numeric_limits<int>::max();
	int index = -1;
	if (x_set.size() > 0) {
		if (x_set[0] < tile.cols - x_set.back() - 1) {
			subdivide.dir = Subdivision::TYPE_LEFT;
			subdivide.dist = x_set[0];
			index = 0;
		}
		else {
			subdivide.dir = Subdivision::TYPE_RIGHT;
			subdivide.dist = tile.cols - x_set.back() - 1;
			index = x_set.size() - 1;
		}
	}
	if (y_set.size() > 0) {
		if (y_set[0] < subdivide.dist && y_set[0] < tile.rows - y_set.back() - 1) {
			subdivide.dir = Subdivision::TYPE_TOP;
			subdivide.dist = y_set[0];
			index = 0;
		}
		else if (tile.rows - y_set.back() - 1 < subdivide.dist) {
			subdivide.dir = Subdivision::TYPE_BOTTOM;
			subdivide.dist = tile.rows - y_set.back() - 1;
			index = y_set.size() - 1;
		}
	}

	// find the dual correspondence
	if (subdivide.dir == Subdivision::TYPE_LEFT) {
		for (int i = 0; i < x_set.size(); ++i) {
			if (i == index) continue;

			if (abs(tile.cols - x_set[i] - 1 - subdivide.dist) < 3) {
				subdivide.dual = true;
				break;
			}
		}
	}
	else if (subdivide.dir == Subdivision::TYPE_RIGHT) {
		for (int i = 0; i < x_set.size(); ++i) {
			if (i == index) continue;

			if (abs(x_set[i] - subdivide.dist) < 3) {
				subdivide.dual = true;
				break;
			}
		}
	}
	else if (subdivide.dir == Subdivision::TYPE_TOP) {
		for (int i = 0; i < y_set.size(); ++i) {
			if (i == index) continue;

			if (abs(tile.rows - y_set[i] - 1 - subdivide.dist) < 3) {
				subdivide.dual = true;
				break;
			}
		}
	}
	else if (subdivide.dir == Subdivision::TYPE_BOTTOM) {
		for (int i = 0; i < y_set.size(); ++i) {
			if (i == index) continue;

			if (abs(y_set[i] - subdivide.dist) < 3) {
				subdivide.dual = true;
				break;
			}
		}
	}

	return true;
}

/**
 * 領域(x1,y1)-(x2,y2)を分割し、領域を更新する。
 */
void updateRegion(const Subdivision& subdivision, int& x1, int& y1, int& x2, int& y2) {
	if (subdivision.dir == Subdivision::TYPE_LEFT) {
		x1 += subdivision.dist;
		if (subdivision.dual) {
			x2 -= subdivision.dist;
		}
	}
	else if (subdivision.dir == Subdivision::TYPE_RIGHT) {
		x2 -= subdivision.dist;
		if (subdivision.dual) {
			x1 -= subdivision.dist;
		}
	}
	else if (subdivision.dir == Subdivision::TYPE_TOP) {
		y1 += subdivision.dist;
		if (subdivision.dual) {
			y2 -= subdivision.dist;
		}
	}
	else if (subdivision.dir == Subdivision::TYPE_BOTTOM) {
		y2 -= subdivision.dist;
		if (subdivision.dual) {
			y1 -= subdivision.dist;
		}
	}
}

/**
 * facadeの各tileを1ステップ、細かく分割し、subdivisionsに分割方法を追加する。
 * 1つのタイルも分割しない場合はfalseを返却する。
 */
bool subdivideFacadeTilesByOneStep(const cv::Mat& img, vector<vector<int>>& y_set, vector<vector<int>>& x_set, vector<vector<vector<Subdivision>>>& subdivisions) {
	vector<vector<vector<Subdivision>>> votes(y_set.size());

	for (int i = 0; i < y_set.size(); ++i) {
		for (int j = 0; j < y_set[i].size() - 1; ++j) {
			int y1 = y_set[i][j];
			int y2 = y_set[i][j + 1];

			votes[i].resize(x_set.size());
			for (int k = 0; k < x_set.size(); ++k) {
				for (int l = 0; l < x_set[k].size() - 1; ++l) {
					int x1 = x_set[k][l];
					int x2 = x_set[k][l + 1];

					int u1 = x1;
					int u2 = x2;
					int v1 = y1;
					int v2 = y2;

					for (int s = 0; s < subdivisions[i][k].size(); ++s) {
						updateRegion(subdivisions[i][k][s], u1, v1, u2, v2);
					}

					cv::Mat tile(img, cv::Rect(u1, v1, u2 - u1 - 1, v2 - v1 - 1));
					Subdivision subdivide;
					if (subdivideTile(tile, subdivide, max(x2 - x1, y2 - y1) * 0.2)) {
						votes[i][k].push_back(subdivide);
					}
				}
			}
		}
	}

	bool subdivided = false;

	// choose the maximum vote for each type of tile
	for (int i = 0; i < votes.size(); ++i) {
		for (int j = 0; j < votes[i].size(); ++j) {
			if (votes[i][j].size() == 0) continue;

			subdivided = true;
						
			float sigma = 3.0f;

			// find the maximum vote for horizontal split
			float x_vote_max;
			Subdivision x_subdivide(0, false, 0);
			{
				vector<float> histogram(x_set[j][1] - x_set[j][0], 0.0f);

				for (int s = 0; s < votes[i][j].size(); ++s) {
					if (votes[i][j][s].dir == Subdivision::TYPE_TOP || votes[i][j][s].dir == Subdivision::TYPE_BOTTOM) continue;

					int dist;
					if (votes[i][j][s].dir == Subdivision::TYPE_LEFT) {
						dist = votes[i][j][s].dist;
					}
					else if (votes[i][j][s].dir == Subdivision::TYPE_RIGHT) {
						dist = x_set[j][1] - x_set[j][0] - votes[i][j][s].dist;
					}

					for (int c = 0; c < x_set[j][1] - x_set[j][0]; ++c) {
						histogram[c] += utils::gause(c - dist, sigma);
					}
				}

				x_vote_max = *max_element(histogram.begin(), histogram.end());
				x_subdivide.dist = distance(histogram.begin(), max_element(histogram.begin(), histogram.end()));

				// select the type
				int min_dist = numeric_limits<int>::max();
				for (int s = 0; s < votes[i][j].size(); ++s) {
					if (votes[i][j][s].dir == Subdivision::TYPE_LEFT) {
						int d = abs(votes[i][j][s].dist - x_subdivide.dist);
						if (d < min_dist) {
							min_dist = d;
							x_subdivide = votes[i][j][s];
						}
					}
					else if (votes[i][j][s].dir == Subdivision::TYPE_RIGHT) {
						int d = abs(x_set[j][1] - x_set[j][0] - votes[i][j][s].dist - x_subdivide.dist);
						if (d < min_dist) {
							min_dist = d;
							x_subdivide = votes[i][j][s];
						}
					}
				}
			}

			// find the maximum vote for vertical split
			float y_vote_max;
			Subdivision y_subdivide(1, false, 0);
			{
				vector<float> histogram(y_set[j][1] - y_set[j][0], 0.0f);

				for (int s = 0; s < votes[i][j].size(); ++s) {
					if (votes[i][j][s].dir == Subdivision::TYPE_LEFT || votes[i][j][s].dir == Subdivision::TYPE_RIGHT) continue;

					int dist;
					if (votes[i][j][s].dir == Subdivision::TYPE_TOP) {
						dist = votes[i][j][s].dist;
					}
					else if (votes[i][j][s].dir == Subdivision::TYPE_BOTTOM) {
						dist = y_set[j][1] - y_set[j][0] - votes[i][j][s].dist;
					}

					for (int r = 0; r < y_set[j][1] - y_set[j][0]; ++r) {
						histogram[r] += utils::gause(r - dist, sigma);
					}
				}

				y_vote_max = *max_element(histogram.begin(), histogram.end());
				y_subdivide.dist = distance(histogram.begin(), max_element(histogram.begin(), histogram.end()));

				// select the type
				int min_dist = numeric_limits<int>::max();
				for (int s = 0; s < votes[i][j].size(); ++s) {
					if (votes[i][j][s].dir == Subdivision::TYPE_TOP) {
						int d = abs(votes[i][j][s].dist - y_subdivide.dist);
						if (d < min_dist) {
							min_dist = d;
							y_subdivide = votes[i][j][s];
						}
					}
					else if (votes[i][j][s].dir == Subdivision::TYPE_BOTTOM) {
						int d = abs(y_set[j][1] - y_set[j][0] - votes[i][j][s].dist - y_subdivide.dist);
						if (d < min_dist) {
							min_dist = d;
							y_subdivide = votes[i][j][s];
						}
					}
				}
			}

			if (x_vote_max > y_vote_max) {
				subdivisions[i][j].push_back(x_subdivide);
			}
			else {
				subdivisions[i][j].push_back(y_subdivide);
			}
		}
	}

	return subdivided;
}

void subdivideFacadeTiles(const cv::Mat& img, vector<vector<int>>& y_set, vector<vector<int>>& x_set, vector<vector<vector<Subdivision>>>& subdivisions) {
	subdivisions.resize(y_set.size());
	for (int i = 0; i < y_set.size(); ++i) {
		subdivisions[i].resize(x_set.size());
	}

	// 各tileを細かく分割する
	while (true) {
		if (!subdivideFacadeTilesByOneStep(img, y_set, x_set, subdivisions)) break;

		outputFacadeAndTileStructure(img, y_set, x_set, subdivisions, "result4b.png");
	}
}

/**
 * Ver(y)の極小値をsplit lineの候補とし、S_max(y)に基づいて最適なsplit lineの組み合わせを探す。
 *
 * @param img			Facade画像 (3-channel image)
 * @param SV_max		S_max(y)
 * @param h_max			h_max(y)
 * @param floor_height	floor height
 * @param Ver			Ver(y)
 * @param y_split		最適なsplit lineの組み合わせ
 */
void findBestHorizontalSplitLines(const cv::Mat& img, const cv::Mat_<float>& SV_max, const cv::Mat_<float>& h_max, const float floor_height, const cv::Mat_<float>& Ver, vector<int>& y_split) {
	y_split.clear();

	vector<int> y_potential;
	for (int r = 0; r < Ver.rows; ++r) {
		if (cvutils::localMinimum(Ver, r, 3)) {
			y_potential.push_back(r);
		}
	}

	vector<int> flags(y_potential.size(), 0);
	float max_score = -numeric_limits<float>::max();
	vector<int> max_flags(y_potential.size(), 0);
	while (true) {
		// compute the score
		float score = 0.0f;
		int y_prev = 0;
		int count = 0;
		bool valid = true;
		for (int i = 0; i < flags.size(); ++i) {
			if (flags[i] == 0) {
				// このsplit lineを使用しないことによるペナルティ
				score -= SV_max(y_potential[i], 0) * 0.5;
				continue;
			}

			count++;

			// １つ前のsplit lineとの距離をチェック
			if (abs(y_potential[i] - y_prev) < 10 || abs(y_potential[i] - y_prev) > floor_height * 3.0f) {
				valid = false;
				break;
			}

			if (abs(y_potential[i] - y_prev - h_max(y_prev, 0)) < 5 || abs(y_potential[i] - y_prev - h_max(y_potential[i], 0)) < 5) {
				score += SV_max(y_potential[i], 0);
			}

			y_prev = y_potential[i];
		}

		if (valid) {
			score /= count;

			// update the max score
			if (score > max_score) {
				max_score = score;
				max_flags = flags;
			}
		}

		// increment the flags
		bool incremented = false;
		for (int i = 0; i < flags.size(); ++i) {
			if (flags[i] == 0) {
				flags[i] = 1;
				incremented = true;
				break;
			}
			else {
				flags[i] = 0;
			}
		}

		if (!incremented) break;
	}

	// y_splitに、最適解を格納する
	for (int i = 0; i < max_flags.size(); ++i) {
		if (max_flags[i] == 1) {
			y_split.push_back(y_potential[i]);
		}
	}
}

/**
* Hor(x)の極小値をsplit lineの候補とし、S_max(x)に基づいて最適なsplit lineの組み合わせを探す。
*
* @param img			Facade画像 (3-channel image)
* @param SH_max			S_max(x)
* @param w_max			w_max(x)
* @param tile_width		tile width
* @param Hor			Hor(x)
* @param x_split		最適なsplit lineの組み合わせ
*/
void findBestVerticalSplitLines(const cv::Mat& img, const cv::Mat_<float>& SH_max, const cv::Mat_<float>& w_max, const float tile_width, const cv::Mat_<float>& Hor, vector<int>& x_split) {
	x_split.clear();

	vector<int> x_potential;
	for (int c = 0; c < Hor.cols; ++c) {
		if (cvutils::localMinimum(Hor, c, 3)) {
			x_potential.push_back(c);
		}
	}

	vector<int> flags(x_potential.size(), 0);
	float max_score = -numeric_limits<float>::max();
	vector<int> max_flags(x_potential.size(), 0);
	while (true) {
		// compute the score
		float score = 0.0f;
		int x_prev = 0;
		int count = 0;
		bool valid = true;
		for (int i = 0; i < flags.size(); ++i) {
			if (flags[i] == 0) {
				// このsplit lineを使用しないことによるペナルティ
				score -= SH_max(0, x_potential[i]) * 0.5;
				continue;
			}

			count++;

			// １つ前のsplit lineとの距離をチェック
			if (abs(x_potential[i] - x_prev) < 10 || abs(x_potential[i] - x_prev) > tile_width * 3.0f) {
				valid = false;
				break;
			}

			if (abs(x_potential[i] - x_prev - w_max(0, x_prev)) < 5 || abs(x_potential[i] - x_prev - w_max(0, x_potential[i])) < 5) {
				score += SH_max(0, x_potential[i]);
			}

			x_prev = x_potential[i];
		}

		if (valid) {
			score /= count;

			// update the max score
			if (score > max_score) {
				max_score = score;
				max_flags = flags;
			}
		}

		// increment the flags
		bool incremented = false;
		for (int i = 0; i < flags.size(); ++i) {
			if (flags[i] == 0) {
				flags[i] = 1;
				incremented = true;
				break;
			}
			else {
				flags[i] = 0;
			}
		}

		if (!incremented) break;
	}

	// x_splitに、最適解を格納する
	for (int i = 0; i < max_flags.size(); ++i) {
		if (max_flags[i] == 1) {
			x_split.push_back(x_potential[i]);
		}
	}
}

void subdivideFacade(const cv::Mat& img) {
	cv::Mat grayImg;
	cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);

	//////////////////////////////////////////////////////////////////////////////
	// obtain edge image
	cv::Mat grad;
	cv::Mat grad_x;
	cv::Mat grad_y;
	cv::Mat abs_grad_x;
	cv::Mat abs_grad_y;
	cv::Sobel(grayImg, grad_x, CV_16S, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);

	//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
	cv::Sobel(grayImg, grad_y, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);

	/// Total Gradient (approximate)
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

	//cv::imwrite("grad.png", grad);
	grad.convertTo(grayImg, CV_8U);
	//cv::imwrite("grad2.png", grayImg);
	//////////////////////////////////////////////////////////////////////////////





	// vertical split
	cv::Mat_<float> SV_max;
	cv::Mat_<float> h_max;
	computeSV(grayImg, SV_max, h_max, make_pair(10, 40));

	// visualize SV_max(y) and h_max(y)
	outputFacadeStructureV(img, SV_max, h_max, "SV_max.png");

	// estimate the floor height
	float floor_height = cvutils::getMostPopularValue(h_max, 3, 3);
	cout << "Floor height: " << floor_height << endl;
	
	// horizontal split
	cv::Mat_<float> SH_max;
	cv::Mat_<float> w_max;
	computeSH(grayImg, SH_max, w_max, make_pair(10, 40));

	// visualize SH_max(x) and w_max(x)
	outputFacadeStructureH(img, SH_max, w_max, "SH_max.png");

	float tile_width = cvutils::getMostPopularValue(w_max, 3, 3);
	cout << "Tile width: " << tile_width << endl;

	// visualize SV_max(y), h_max(y), SH_max(x) and w_max(x)
	outputFacadeStructure(img, SV_max, h_max, SH_max, w_max, "SV_SH_max.png");

	// compute Ver(y) and Hor(x)
	cv::Mat_<float> Ver;
	cv::Mat_<float> Hor;
	computeVerAndHor(img, Ver, Hor, floor_height * 0.1);
	
	// potentialのsplit lineの中で、最適な組み合わせを探す
	vector<int> y_split;
	findBestHorizontalSplitLines(img, SV_max, h_max, floor_height, Ver, y_split);	
	outputFacadeStructureV(img, SV_max, h_max, y_split, "facade_structure_V.png");
	
	// potentialのsplit lineの中で、最適な組み合わせを探す
	vector<int> x_split;
	findBestVerticalSplitLines(img, SH_max, w_max, tile_width, Hor, x_split);
	outputFacadeStructureH(img, SH_max, w_max, x_split, "facade_structure_H.png");

	// visualize the facade structure
	outputFacadeStructure(img, SV_max, h_max, SH_max, w_max, y_split, x_split, "facade_structure.png");


	/////////////////////////////////////////////////////////////////
	// DEBUG
	cvutils::outputImageWithHorizontalAndVerticalGraph(img, Ver, Hor, "facade_subdivision_by_ver_hor.png", cvutils::GRAPH_LOCAL_MINIMUM, 1);
	/////////////////////////////////////////////////////////////////



	/*
	// subdivide IF
	cv::Mat_<float> Ver;
	cv::Mat_<float> Hor;
	computeVerAndHor(imgIF, Ver, Hor, floor_height * 0.1);
	cvutils::outputImageWithHorizontalAndVerticalGraph(imgIF, Ver, Hor, "IF_subdivision_by_ver_hor.png", cvutils::GRAPH_LOCAL_MINIMUM, 1);

	// get the best subdivision on IF
	vector<int> y_split_IF;
	vector<int> x_split_IF;
	subdivideByVerAndHor(Ver, Hor, y_split_IF, x_split_IF);

	// expand the subdivision to the full facade
	vector<int> y_split;
	vector<int> x_split;
	expandSubdivisionToFullFacade(y_set, x_set, y_split_IF, x_split_IF, y_split, x_split);

	outputFacadeStructure(img, y_split, x_split, "result3.png");
	*/


	//outputFacadeStructure(img, SV_max, h_max, SH_max, w_max, y_set, x_set, "result3.png");

	/*
	vector<vector<vector<Subdivision>>> subdivisions;
	subdivideFacadeTiles(img, y_set, x_set, subdivisions);
	outputFacadeAndTileStructure(img, y_set, x_set, subdivisions, "result4.png");
	*/
}

int main() {
	cv::Mat img = cv::imread("../facade_small/facade1.png");

	subdivideFacade(img);

	return 0;
}
