#include "stdafx.h"
#include "detect.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_REG 1000
#define EPS 0.00001
#define PI 3.141592

static char jpg_path[100];

RNG rng(12345);
Point res_pts[2][100];
int ntray = 0;
int n_tray_pts[2] = { 0, 0 };

int x_dist = 0;
int nrow_split = -1;

//---------------functions for image labeling-----------------------------------
//structure for image labeling
struct afeature {
	double area; //region's area
	double cx; // x coordinate of region's  center point
	double cy; //y coordinate of region's  center point
	int minw; // region's minimum x 
	int maxw;// region's maximum x 
	int minh;// region's minimum y 
	int maxh;// region's maximum y 
	Point pminH; // region's point which has minimum y coordinates
	Point pmaxH; // region's point which has maximum y coordinates
	Point pminW; // region's point which has minimum x coordinates
	Point pmaxW; // region's point which has maximum x coordinates
};

//global data for image labeling
short *l_data;

//sub function for image labeling
int UpdateIndex(int *index, int nind) {

	if (index[nind] == 0)
		return nind;

	int k = index[nind];
	index[nind] = UpdateIndex(index, k);
}

//main function for image labeling
static void findComponents2(short *data, int label, int W, int H) {
	int index[MAX_REG];
	short *pd = data;
	int l = 0, u = 0;
	int minv, maxv;

	//index initialize
	memset(index, 0x00, sizeof(int) * MAX_REG);

	//first scan
	int cLabel = 0;
	for (int i = 0; i < H; i++){
		for (int j = 0; j < W; j++, pd++) {
			if (*pd == 0)
				continue;

			l = u = 0;
			if (i > 0) u = pd[-W];
			if (j > 0) l = pd[-1];
			minv = MIN(l, u);
			maxv = MAX(l, u);

			if (minv > 0 && maxv != minv) {
				index[maxv] = minv;//index insert
				*pd = minv;
			}
			else if (minv == 0 && maxv != 0)
				*pd = maxv;
			else if (minv == maxv && minv != 0)
				*pd = maxv;
			else {
				cLabel++;
				*pd = cLabel;
			}
		}
	}

	//index dictionary update
	for (int i = cLabel; i > 0; i--)
		UpdateIndex(index, i);

	int count = 0;
	for (int i = 0; i < cLabel; i++) {
		if (index[i] > 0)count++;
	}

	//second scan
	pd = data;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++, pd++) {
			minv = *pd;
			if (minv == 0)
				continue;

			if (index[minv] != 0)
				*pd = index[minv];
		}
	}

	//order labeled image values
	pd = data;
	memset(index, 0x00, sizeof(int) * MAX_REG);
	cLabel = 0;

	short v;
	for (int i = 0; i < W * H; i++, pd++) {
		v = *pd;
		if (v == 0)
			continue;

		int cval = -1;
		for (int j = 0; j < cLabel; j++) {
			if (index[j] == v) {
				cval = j + 1;
				break;
			}
		}

		if (cval != -1) {
			*pd = cval;
			continue;
		}
		//new create
		index[cLabel++] = v;
		*pd = cLabel;
	}
	cLabel = cLabel;
	cLabel = cLabel - 1;
}
void showShort(short *data, int W, int H) {
	Mat res;
	res.create(Size(W, H), CV_8UC3);
	memset(res.data, 0x00, W*H * 3);

	//generate random color
	RNG rng(12345);
	uchar clr[4096][3];
	for (int i = 0; i < 4096; i++) {
		clr[i][0] = rng.uniform(0, 255);
		clr[i][1] = rng.uniform(0, 255);
		clr[i][2] = rng.uniform(0, 255);
	}

	unsigned char *pu = res.data;
	short *pd = data;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++, pu += 3, pd++) {
			int v = *pd;
			*pu = clr[v][0];
			*(pu + 1) = clr[v][1];
			*(pu + 2) = clr[v][2];
		}
	}

	imshow("show", res);
	waitKey(0);
}
//get segment's area, width, height, center
//input image must be 0, 255 value
static int get_label_info(Mat _img, afeature **af) {
	int label;
	Mat simg;
	uchar *pu, vu;
	short *ps, v;
	int r, c, W, H;
	afeature *f;
	afeature *pf;


	W = _img.cols;
	H = _img.rows;

	simg.create(Size(W, H), CV_16SC1);
	memset(simg.data, 0x00, W * H * sizeof(short));

	for (r = 0; r < H; r++) {
		pu = (uchar*)_img.data + r * W;
		ps = (short*)simg.data + r * W;
		for (c = 0; c < W; c++, pu++, ps++) {
			vu = *pu;
			if (vu > 0 && r != 0 && r != H - 1 && c != 0 && c != W - 1)
				*ps = -1;
			else
				*ps = 0;
		}
	}


	label = 0;
	ps = (short*)simg.data;
	findComponents2(ps, label, W, H);
	//showShort((short*)simg.data, W, H);



	f = (afeature*)malloc(sizeof(afeature) * MAX_REG);
	memset(f, 0x00, sizeof(afeature) * MAX_REG);
	for (r = 0; r < MAX_REG; r++) {
		f[r].minh = H;
		f[r].minw = W;
	}

	label = 0;


	for (r = 0; r < H; r++) {
		for (c = 0; c < W; c++, ps++){
			v = *ps;
			if (v == 0 || v == -1)
				continue;

			pf = &f[v];
			pf->area++;
			pf->cx += c;
			pf->cy += r;
			//pf->minh = min(pf->minh, r);
			if (pf->minh > r) {
				pf->minh = r;
				pf->pminH.x = c;
				pf->pminH.y = r;
			}
			//pf->maxh = max(pf->maxh, r);
			if (pf->maxh < r) {
				pf->maxh = r;
				pf->pmaxH.x = c;
				pf->pmaxH.y = r;
			}
			//pf->minw = min(pf->minw, c);
			if (pf->minw > c) {
				pf->minw = c;
				pf->pminW.x = c;
				pf->pminW.y = r;
			}
			//pf->maxw = max(pf->maxw, c);
			if (pf->maxw < c) {
				pf->maxw = c;
				pf->pmaxW.x = c;
				pf->pmaxW.y = r;
			}
			label = MAX(label, v);
		}
	}

	for (r = 1; r <= label; r++) {
		f[r].cx = f[r].cx / f[r].area;
		f[r].cy = f[r].cy / f[r].area;
	}
	*af = f;
	simg.release();
	return label;
}

// ------------------ end of labeling, start new function -----------------------
//------   geometry functions -----------

// return angle in radian between line of points and x axis
float get_angle_from_pts(Point pt1, Point pt2) {
	float x = pt2.x - pt1.y;
	float y = pt2.y - pt1.y;
	if (x < EPS) {
		if (y > 0) return PI / 2;
		else return -PI / 2;
	}
	return atan(y / x);
}

// return the straightness value for three points
int get_straightness(Point pt1, Point pt2, Point pt3) {

	float error = PI / 8;
	float ang1 = get_angle_from_pts(pt1, pt2);
	float ang2 = get_angle_from_pts(pt1, pt3);
	float ang3 = get_angle_from_pts(pt2, pt3);
	//ang1 = abs(ang1);
	//ang2 = abs(ang2);
	if (abs(ang1 - ang2) < error || abs(abs(ang1 + ang2) - PI / 2) < error)
		if (abs(ang3 - ang2) < error || abs(abs(ang3 + ang2) - PI / 2) < error)
			return 1;
	return 0;
}

//calculate distance from pt0 to (pt1, pt2)line
float dist_line_point(Point pt1, Point pt2, Point pt0) {
	float x1 = pt1.x;
	float y1 = pt1.y;
	float x2 = pt2.x;
	float y2 = pt2.y;
	float x0 = pt0.x;
	float y0 = pt0.y;

	return abs((x2 - x1)*(y1 - y0) - (x1 - x0)*(y2 - y1)) / sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

//calculate the distance between line (pt0, pt1) and line(pt2, pt3)
float dist_line_line(Point pt0, Point pt1, Point pt2, Point pt3) {
	float dis1 = dist_line_point(pt0, pt1, pt2);
	float dis2 = dist_line_point(pt0, pt1, pt3);
	return (dis1 + dis2) / 2;
}

bool sort_from_x(Point *pts, int *row_idxs, int *npts) {

	int min_x = 65;
	int max_x = 120;
	if (*npts > 4){
		min_x = 50;
		max_x = 110;
	}
	else{

	}

	int error = 15;

	int new_idx[10];
	int dists[10];

	int npt = *npts;

	if (npt < 3)
		return false;

	// get leftest point
	Point lpt = Point(1000, 1000);
	for (int i = 0; i < npt; i++) {
		if (lpt.x > pts[row_idxs[i]].x)
			lpt = pts[row_idxs[i]];
	}

	// calc dists from left point
	for (int i = 0; i < npt; i++) {
		int idx = row_idxs[i];
		int dist = sqrt((lpt.x - pts[idx].x) * (lpt.x - pts[idx].x) + (lpt.y - pts[idx].y) * (lpt.y - pts[idx].y));
		dists[i] = dist;
	}
	// sort dists and row_idxs
	for (int i = 0; i < npt-1; i++) {
		for (int j = i + 1; j < npt; j++) {
			if (dists[i] > dists[j]) {
				int tmp = dists[i];
				dists[i] = dists[j];
				dists[j] = tmp;

				tmp = row_idxs[i];
				row_idxs[i] = row_idxs[j];
				row_idxs[j] = tmp;
			}
		}
	}

	// calc mean dist for normal
	int m_dist = 0;
	int nsumed = 0;

	for (int i = 0; i < npt-1; i++) {
		int idx0 = row_idxs[i];
		int idx1 = row_idxs[i + 1];
		int dist = sqrt((pts[idx0].x - pts[idx1].x)*(pts[idx0].x - pts[idx1].x) + (pts[idx0].y - pts[idx1].y)*(pts[idx0].y - pts[idx1].y));

		dists[i] = dist;
		if (dist > min_x && dist < max_x) {
			m_dist += dist;
			nsumed++;
		}
	}

	if (nsumed == 0)
		return false;

	m_dist /= nsumed;
	x_dist = m_dist;

	// remove the incorrect points
	int remove_idx[10];
	int n_remove = 0;
	for (int i = 0; i < npt - 1; i++) {
		if (dists[i] < error) {
			remove_idx[n_remove++] = i;
			continue;
		}
		if (abs(dists[i] - m_dist) < error || abs(dists[i] - m_dist * 2) < 2 * error)
			continue;
		if (i == 0) {
			// remove the first point
			remove_idx[n_remove++] = 0;
		}
		else if (i == npt - 1) {
			//remove the last point
			remove_idx[n_remove++] = npt - 1;
		}
		else
			remove_idx[n_remove++] = i + 1;
	}

	// remove the idxs to remove
	for (int i = n_remove-1; i >=0; i--) {
		int rm_id = remove_idx[i];
		for (int j = rm_id; j < npt - 1; j++) {
			row_idxs[j] = row_idxs[j + 1];
		}
		*npts = npt-1;
		npt--;
	}

	if (npt < 3)
		return false;

	return true;
}

void split_insert_pts(Point *pts, int *n_pts, int *idxs, int *n_idxs, int nrow) {
	int startx = 1000;
	int endx = 0;
	
	
	int error_y = 10;
	

	nrow_split = 0;


	// remove one point that has been special large value
	if (1){
		int first_second_delta = 30;
		int m1 = 1000, m2 = 1000;
		int im1 = -1, im2 = -1;
		for (int i = 0; i < nrow; i++) {
			int s = pts[idxs[i * 10]].x;
			int e = pts[idxs[i * 10 + n_idxs[i] - 1]].x;

			if (m1 > s) {
				m1 = s;
				im1 = i;
			}
			if (m2 > s && i != im1) {
				m2 = s;
				im2 = i;
			}
		}
		if (m2 - m1 > first_second_delta) {
			// remove im1 (first item in the ith row)
			for (int i = 1; i < n_idxs[im1]; i++)
				idxs[10*im1 +i -1] = idxs[10*im1 + i];
			n_idxs[im1]--;
		}

		m1 = 0, m2 = 0;
		im1 = -1, im2 = -1;
		for (int i = 0; i < nrow; i++) {
			int s = pts[idxs[i * 10]].x;
			int e = pts[idxs[i * 10 + n_idxs[i] - 1]].x;

			if (m1 < e) {
				m1 = e;
				im1 = i;
			}
			if (m2 < e && i != im1) {
				m2 = e;
				im2 = i;
			}
		}
		if (m1 - m2 > first_second_delta) {
			// remove im1 (first item in the ith row)
			n_idxs[im1]--;
		}
	}

	// calc dists between rows
	float r_dists[15];
	for (int i = 0; i < nrow-1; i++) {
		int id0 = idxs[i * 10];
		int id1 = idxs[i * 10 + n_idxs[i] - 1];
		int id2 = idxs[(i+1) * 10];
		int id3 = idxs[(i+1) * 10 + n_idxs[i+1] - 1];
		r_dists[i] = dist_line_line(pts[id0], pts[id1], pts[id2], pts[id3]);
	}
	// get the row id that has different y-width
	float r_dist0 = r_dists[1];
	for (int i = 2; i < nrow - 1; i++) {
		if (abs(r_dist0 - r_dists[i]) > error_y) {
			nrow_split = i + 1;
			break;
		}
	}
	// skip last row
	if (nrow_split == nrow - 1)
		nrow_split = nrow;


	// first tray --------------------------------------------
	// get startx, endx for first tray
	for (int i = 0; i < nrow_split; i++) {
		Point xs = pts[idxs[i * 10]];
		Point xe = pts[idxs[i * 10 + n_idxs[i] - 1]];
		
		if (startx > xs.x)
			startx = xs.x;
		if (endx < xe.x)
			endx = xe.x;
	}
	// insert the eliminated points
	for (int i = 0; i < nrow_split; i++) {

		int delta_x = 20;
		if (n_idxs[i] > 4)
			delta_x = 10;

		// determine the start part
		Point pt0 = pts[idxs[i * 10]], pt1 = pts[idxs[i*10 + 1]];
		int k = round(norm(pt1 - pt0) / x_dist);

		int d = pt0.x - startx;
		if (d > 2 * x_dist - delta_x) {
			for (int j = 1; j >= 0; j--) {
				int x = (float)(k + j + 1) / (float)k * pt0.x - (float)(j + 1) / (float)k * pt1.x;
				int y = (float)(k + j + 1) / (float)k * pt0.y - (float)(j + 1) / (float)k * pt1.y;
				
				// insert point to pts
				pts[*n_pts] = Point(x, y);
				*n_pts = *n_pts + 1;

				// insert index of point to idxs
				for (int kk = n_idxs[i] - 1; kk >= 0; kk--) {
					idxs[i * 10 + kk + 1] = idxs[i * 10 + kk];
				}
				idxs[i * 10] = *n_pts - 1;
				n_idxs[i]++;
			}
		}
		else if (d > x_dist - delta_x) {
			int x = (float)(k + 1) / (float)k * pt0.x - 1.0 / (float)k * pt1.x;
			int y = (float)(k + 1) / (float)k * pt0.y - 1.0 / (float)k * pt1.y;

			// insert point to pts
			pts[*n_pts] = Point(x, y);
			*n_pts = *n_pts + 1;

			// insert index of point to idxs
			for (int kk = n_idxs[i] - 1; kk >= 0; kk--) {
				idxs[i * 10 + kk + 1] = idxs[i * 10 + kk];
			}
			idxs[i * 10] = *n_pts - 1;
			n_idxs[i]++;
		}

		// determine the last part
		pt0 = pts[idxs[i * 10 + n_idxs[i] - 1]];
		pt1 = pts[idxs[i * 10 + n_idxs[i] - 2]];

		k = round(norm(pt0 - pt1) / x_dist);

		d = endx - pt0.x;
		if (d > 2 * x_dist - 15) {
			for (int j = 0; j < 2; j++) {
				int x = (float)(k + j + 1) / (float)k * pt0.x - (float)(j + 1) / (float)k * pt1.x;
				int y = (float)(k + j + 1) / (float)k * pt0.y - (float)(j + 1) / (float)k * pt1.y;

				// insert point to pts
				pts[*n_pts] = Point(x, y);
				*n_pts = *n_pts + 1;

				// insert index of point to idxs
				idxs[i * 10 + n_idxs[i]] = *n_pts - 1;
				n_idxs[i]++;
			}
		}
		else if (d > x_dist - 15) {
			int x = (float)(k + 1) / (float)k * pt0.x - 1.0 / (float)k * pt1.x;
			int y = (float)(k + 1) / (float)k * pt0.y - 1.0 / (float)k * pt1.y;

			// insert point to pts
			pts[*n_pts] = Point(x, y);
			*n_pts = *n_pts + 1;

			// insert index of point to idxs
			idxs[i * 10 + n_idxs[i]] = *n_pts - 1;
			n_idxs[i]++;
		}

		// insert intermediate points
		int n_insert = 0;

		for (int j = 0; j < n_idxs[i]-1; j++) {
			pt0 = pts[idxs[i * 10 + j]];
			pt1 = pts[idxs[i * 10 + j + 1]];

			k = round(norm(pt1-pt0) / (double)x_dist);

			for (int kk = 1; kk < k; kk++) {
				int m = kk, n = k - kk;
				int x = (float)n / (float)k * pt0.x + (float)m / (float)k * pt1.x;
				int y = (float)n / (float)k * pt0.y + (float)m / (float)k * pt1.y;

				//insert point to pts
				pts[*n_pts] = Point(x, y);
				*n_pts = *n_pts + 1;

				// insert index to pts
				idxs[i * 10 + n_idxs[i] + n_insert] = *n_pts - 1;
				n_insert++;
			}
		}
		n_idxs[i] += n_insert;
	}





	// second tray --------------------------------------------


	// get startx, endx for first tray
	for (int i = nrow_split; i < nrow; i++) {
		Point xs = pts[idxs[i * 10]];
		Point xe = pts[idxs[i * 10 + n_idxs[i] - 1]];

		if (startx > xs.x)
			startx = xs.x;
		if (endx < xe.x)
			endx = xe.x;
	}
	// insert the eliminated points
	for (int i = nrow_split; i < nrow; i++) {

		int delta_x = 20;
		if (n_idxs[i] > 4)
			delta_x = 8;

		// determine the start part
		Point pt0 = pts[idxs[i * 10]], pt1 = pts[idxs[i * 10 + 1]];
		int k = round(norm(pt1 - pt0) / x_dist);

		int d = pt0.x - startx;
		if (d > 2 * x_dist - delta_x) {
			for (int j = 1; j >= 0; j--) {
				int x = (float)(k + j + 1) / (float)k * pt0.x - (float)(j + 1) / (float)k * pt1.x;
				int y = (float)(k + j + 1) / (float)k * pt0.y - (float)(j + 1) / (float)k * pt1.y;

				// insert point to pts
				pts[*n_pts] = Point(x, y);
				*n_pts = *n_pts + 1;

				// insert index of point to idxs
				for (int kk = n_idxs[i] - 1; kk >= 0; kk--) {
					idxs[i * 10 + kk + 1] = idxs[i * 10 + kk];
				}
				idxs[i * 10] = *n_pts - 1;
				n_idxs[i]++;
			}
		}
		else if (d > x_dist - delta_x) {
			int x = (float)(k + 1) / (float)k * pt0.x - 1.0 / (float)k * pt1.x;
			int y = (float)(k + 1) / (float)k * pt0.y - 1.0 / (float)k * pt1.y;

			// insert point to pts
			pts[*n_pts] = Point(x, y);
			*n_pts = *n_pts + 1;

			// insert index of point to idxs
			for (int kk = n_idxs[i] - 1; kk >= 0; kk--) {
				idxs[i * 10 + kk + 1] = idxs[i * 10 + kk];
			}
			idxs[i * 10] = *n_pts - 1;
			n_idxs[i]++;
		}

		// determine the last part
		pt0 = pts[idxs[i * 10 + n_idxs[i] - 1]];
		pt1 = pts[idxs[i * 10 + n_idxs[i] - 2]];

		k = round(norm(pt0 - pt1) / x_dist);

		d = endx - pt0.x;
		if (d > 2 * x_dist - 15) {
			for (int j = 0; j < 2; j++) {
				int x = (float)(k + j + 1) / (float)k * pt0.x - (float)(j + 1) / (float)k * pt1.x;
				int y = (float)(k + j + 1) / (float)k * pt0.y - (float)(j + 1) / (float)k * pt1.y;

				// insert point to pts
				pts[*n_pts] = Point(x, y);
				*n_pts = *n_pts + 1;

				// insert index of point to idxs
				idxs[i * 10 + n_idxs[i]] = *n_pts - 1;
				n_idxs[i]++;
			}
		}
		else if (d > x_dist - 15) {
			int x = (float)(k + 1) / (float)k * pt0.x - 1.0 / (float)k * pt1.x;
			int y = (float)(k + 1) / (float)k * pt0.y - 1.0 / (float)k * pt1.y;

			// insert point to pts
			pts[*n_pts] = Point(x, y);
			*n_pts = *n_pts + 1;

			// insert index of point to idxs
			idxs[i * 10 + n_idxs[i]] = *n_pts - 1;
			n_idxs[i]++;
		}

		// insert intermediate points
		int n_insert = 0;

		for (int j = 0; j < n_idxs[i] - 1; j++) {
			pt0 = pts[idxs[i * 10 + j]];
			pt1 = pts[idxs[i * 10 + j + 1]];

			k = round(norm(pt1 - pt0) / (double)x_dist);

			for (int kk = 1; kk < k; kk++) {
				int m = kk, n = k - kk;
				int x = (float)n / (float)k * pt0.x + (float)m / (float)k * pt1.x;
				int y = (float)n / (float)k * pt0.y + (float)m / (float)k * pt1.y;

				//insert point to pts
				pts[*n_pts] = Point(x, y);
				*n_pts = *n_pts + 1;

				// insert index to pts
				idxs[i * 10 + n_idxs[i] + n_insert] = *n_pts - 1;
				n_insert++;
			}
		}
		n_idxs[i] += n_insert;
	}
}
// process the prior points
void sort_pts(Mat img, Point *pts, int n_pts) {
// 	int appr_x = 80;
// 	int appr_y = 50;
	int nei_y = 20;

	// ---------- data structure for indexing -------------------------------------------
	// marking the state of points to be indexed
	int mark[100];
	memset(mark, 0x00, sizeof(int) * 100);

	// indices of points in each row
	int idxs[150]; // 10 * 15
	for (int i = 0; i < 150; i++)
		idxs[i] = -1;

	// middle y values in each row
	int midy[15];
	memset(midy, 0x00, sizeof(int) * 15);

	// number of points in each row
	int n_idxs[15];
	memset(n_idxs, 0x00, sizeof(int) * 15);

	// number of rows
	int nrows = 0;
	// ------------------------------------------------------------------------------

	// processing each points
	for (int i = 0; i < n_pts; i++) {
		// if indexed, continue
		if (mark[i]) continue;

		// current processing point
		int x = pts[i].x; 
		int y = pts[i].y;

		// insert to the idxs
		for (int j = 0; j < 15; j++) {

			if (midy[j] == 0) {
				//insert idx first in this row
				idxs[j * 10] = i;
				midy[j] = y;
				n_idxs[j] += 1;
				nrows++;
				break;
			}
			else if (abs(midy[j] - y) < nei_y) {
				//insert idx in the row that has already elements
				idxs[10 * j + n_idxs[j]] = i;
				midy[j] = (n_idxs[j] * midy[j] + y) / (n_idxs[j] + 1);
				n_idxs[j]++;
				break;
			}
		}
	}

	
	// draw the points
	Mat clr_img;
	cvtColor(img, clr_img, CV_GRAY2BGR);

	// get removed row points
	int med_idxs[150];
	int med_n_idxs[15];
	int med_nrows = 0;

	for (int i = 0; i < nrows; i++) {
		bool brow = sort_from_x(pts, &idxs[i * 10], &n_idxs[i]);
		if (!brow)
			continue;
		memcpy(&med_idxs[med_nrows * 10], &idxs[i * 10], sizeof(int)* n_idxs[i]);
		med_n_idxs[med_nrows++] = n_idxs[i];
	}

	// split trays and insert the points that has been eliminated by eroding

	split_insert_pts(pts, &n_pts, med_idxs, med_n_idxs, med_nrows);

	
	int dist_seperate = 60;
	int ldist = -1;
	if (med_nrows > 0) {
		Point pt1 = pts[med_idxs[0]], pt2 = pts[med_idxs[med_n_idxs[0] - 1]];
		ldist = (pt1.y + pt2.y) / 2;
	}


	for (int i = 0; i < med_nrows; i++) {
		Scalar clr = Scalar(0, 255, 0);
		if (i % 2)
			clr = Scalar(255, 0, 0);
		if (i == 0 && ldist > dist_seperate)
			clr = Scalar(0, 0, 255);

		for (int j = 0; j < med_n_idxs[i]; j++){
			circle(clr_img, pts[med_idxs[i * 10 + j]], 8, clr, 2);
		}
	}
	imwrite(IMG_PATH, clr_img);

//  	imshow("sorted", clr_img);
//  	waitKey(0);

}

// ------------------for depth image------------------
Mat tobyteimg(Mat fimg) {
	float *pdata;
	int i, j, k;
	int w = fimg.cols;
	int h = fimg.rows;
	float minv = 1000000, maxv = 0;
	uchar *pb;
	Mat res;

	pdata = (float*)fimg.data;

	for (i = 0; i < w * h; i++, pdata++) {
		float v = *pdata;
		if (v < EPS) continue;
		if (v < minv) minv = v;
		if (v > maxv) maxv = v;
	}

	float maxv2 = 0;
	pdata = (float*)fimg.data;
	for (i = 0; i < w * h; i++, pdata++) {
		float v = *pdata;
		if (v < EPS) continue;
		if (v > maxv2 && v < maxv-10) maxv2 = v;
	}

	float delta = (maxv2 - minv) / 255.f;

	res.create(fimg.size(), CV_8U);
	pb = res.data;
	pdata = (float*)fimg.data;
	for (i = 0; i < w*h; i++, pb++, pdata++) {
		if (*pdata < minv) {
			*pb = 0;
			continue;
		}
		float val = MIN(255.0, ((*pdata) - minv) / delta);
		*pb = (uchar)(val);
	}

	if (SHOW_PROC) {
		imshow("byte_img", res);
		waitKey(0);
	}
	
	return res;
}


char* txt2jpg(char *txt_path, int w, int h) {
	float *pf;
	Mat fmat, bmat;
	float v;
	char tstr[64000];

	

	freopen(txt_path, "r", stdin);

	fmat.create(Size(640, 480), CV_32F);

	for (int i = 0; i < h; i++) {
		pf = (float *)fmat.data + i * w;
		scanf("%s", tstr);
		int l = strlen(tstr);
		char *token = strtok(tstr, ",");
		
		int cnt = 0;
		for (;;) {
			float v = atof(token);
			*pf = v;
			pf++;
			cnt++;
			if (cnt > 700) {
				cnt = cnt;
			}
			token = strtok(NULL, ",");
			if (token == NULL)
				break;
		}
		cnt = cnt;
	}

	bmat = tobyteimg(fmat);
	imwrite(IMG_PATH, bmat);

	// get jpg path
	int l = strlen(txt_path);
 	txt_path[l - 3] = 'J';
 	txt_path[l - 2] = 'P';
 	txt_path[l - 1] = 'G';
 	strcpy(jpg_path, txt_path);

	// ------------
	return IMG_PATH;
}

void get_idxs(int cth, int w, int h, int nangle, int r, int *idxs) {
	int dx = 0, dy = 0;

	if (nangle == 0) {
		dx = 1; dy = 0;
	}
	else if (nangle == 1) {
		dx = 1; dy = -1;
	}
	else if (nangle == 2) {
		dx = 0; dy = -1;
	}
	else if (nangle == 3) {
		dx = -1; dy = -1;
	}
	else if (nangle == 4) {
		dx = -1; dy = 0;
	}
	else if (nangle == 5) {
		dx = -1; dy = 1;
	}
	else if (nangle == 6) {
		dx = 0; dy = 1;
	}
	else if (nangle == 7) {
		dx = 1; dy = 1;
	}
	
	for (int i = 0; i < r; i++) {
		idxs[i] = cth + dx * i + dy * i * w;
	}
	
}

// filter the depth image
// depth image contains noise depth in the edge and background area
static void filter_depth(Mat depth, int tray_start = 100, int tray_end = 620) {
	int narrow_height = 10;
	float max_scale_rate = 1.4;

	int w = depth.cols;
	int h = depth.rows;

	// edge filtering
	for (int r = 0; r < h; r++) {
		uchar *prow = depth.data + r * w;
		memset(prow, 0x00, sizeof(uchar) * tray_start);
		memset(prow + tray_end, 0x00, sizeof(uchar) * (w - tray_end));
	}
	
	// remove small area
	Mat depth_copy;
	depth_copy.create(depth.size(), CV_8U);
	memcpy(depth_copy.data, depth.data, sizeof(uchar) * w * h);
	medianBlur(depth_copy, depth_copy, 5);
	imshow("copy", depth_copy);
	waitKey(0);
	destroyAllWindows();
	
	vector<vector<Point>> cnts;
	//vector<Vec4i> hierarchy;
	//findContours(depth_copy, cnts, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	findContours(depth_copy, cnts, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	
//	cnts.pop_back();
	cnts.clear();
 	depth_copy.release();
	
 	return;
	
	// approximate contours
	vector<vector<Point>> poly_cnts(cnts.size());
	vector<Rect> boundRect(cnts.size());
	vector<Rect> boundRect2;
	
	for (int i = 0; i < cnts.size(); i++) {
		approxPolyDP(Mat(cnts[i]), poly_cnts[i], 3, true);
		boundRect[i] = boundingRect(Mat(poly_cnts[i]));
	}

	
	int nrect = 0;
	for (int i = 0; i < boundRect.size(); i++) {
		Rect cur = boundRect[i];
		if (cur.height < narrow_height) continue;
		float rate = ((float)cur.height) / ((float)cur.width);
		if ( rate > max_scale_rate) continue;
		boundRect2.push_back(cur);
		rectangle(depth_copy, cur, Scalar(255, 255, 255));
		nrect++;
	}

	

	// remove the area that has narrow width

	imshow("rect image", depth_copy);
	waitKey(0);

	// destroyAllWindows();

	
}

static void filter_depth2(Mat depth, Mat org_img, int tray_start = 160, int tray_end = 580) {
	Point pkt_centers[100];
	int n_pkt = 0;

	Mat erd_img;
	int narrow_height = 10;

	int w = depth.cols;
	int h = depth.rows;

	// edge filtering
	for (int r = 0; r < h; r++) {
		uchar *prow = depth.data + r * w;
		memset(prow, 0x00, sizeof(uchar) * tray_start);
		memset(prow + tray_end, 0x00, sizeof(uchar) * (w - tray_end));
	}


	// erode the depth image to eliminate the noises
	medianBlur(depth, depth, 5);
	Mat mask = getStructuringElement(MORPH_RECT, Size(narrow_height, narrow_height), Point(1, 1));
	erode(depth, erd_img, mask);

	imshow("erode and cut area", depth);

	// get centers of regions
	afeature *af;
	int nClass = get_label_info(erd_img, &af);
	for (int i = 1; i <= nClass; i++) {
		pkt_centers[n_pkt++] = Point(af[i].cx, af[i].cy);
	}
	free(af);

	
	

	sort_pts(org_img, pkt_centers, n_pkt);
	return;
	
	for (int i = 0; i < nClass; i++) {
		circle(erd_img, pkt_centers[i], 8, Scalar(255, 255, 255), 2);
	}



	imshow("erode", erd_img);
	waitKey(0);

}



static void detect_pocket(int thres = 110) {
	Mat img, simg, srimg, rimg;
	int w, h;
	int sw = 16 * 5, sh = 12 * 5;
	int rad = 30;
	int rad2 = 15;

	int idxs[100];

	img = imread(IMG_PATH, CV_LOAD_IMAGE_GRAYSCALE);
	medianBlur(img, img, 5);
	w = img.cols;
	h = img.rows;

	resize(img, simg, Size(sw, sh));

	srimg.create(simg.size(), CV_8U);
	rimg.create(img.size(), CV_8U);
	memset(srimg.data, 0x00, sizeof(uchar)*sw*sh);
	memset(rimg.data, 0x00, sizeof(uchar)*w*h);


	uchar *pb = img.data;
	uchar *pr = rimg.data;
	for (int r = rad; r < h - rad; r++) {
		for (int c = rad; c < w - rad; c++) {
			
			int ncur = r * w + c;
			int cnt = 0;

			for (int i = 0; i < 8; i++) {
				get_idxs(ncur, w, h, i, rad, idxs);
				uchar prevv = pb[idxs[0]];
				for (int j = 0; j < rad; j++) {
					uchar curv = pb[idxs[j]];
					if (curv < prevv) cnt++;
					if (j % 2 == 0)
						prevv = curv;
				}
			}
			pr[ncur] = cnt > thres ? cnt : 0;
		}
	}

	imshow("original image", img);
	imshow("concaved area", rimg);
	

	// filtering the result ---------------------------

// 	vector<vector<Point>> cnts;
// 	vector<Vec4i> hierarchy;
// 	findContours(rimg, cnts, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	filter_depth2(rimg, img);

}

void test() {
	Mat img = imread(IMG_PATH, CV_LOAD_IMAGE_GRAYSCALE);
	blur(img, img, Size(3, 3));
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	Canny(img, canny_output, 40, 80, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
	}

	/// Show in a window
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", drawing);
	waitKey(0);
}

char* action(char *file) {
	detect_pocket();
	return IMG_PATH;
}

