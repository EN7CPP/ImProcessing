#include <iostream>
#include <opencv2/opencv.hpp>
#include "ImageGrid.h"

ImageGrid::ImageGrid(const cv::Mat & img) {
	this->height = img.rows;
	this->width = img.cols;
	this->first_row = new Node*[img.cols];
	this->head=new Node();
	Node * current_col_start = head;
	Node * current = head;
	Node * prev = nullptr;
	
	for (int j = 0; j < img.cols; j++) {
		if (j == img.cols - 1) {
			current->right = nullptr;
		}
		else {
			current->right = new Node();
		}
		current_col_start = current_col_start->right;
		for (int i = 0; i < img.rows; i++) {
			current->up = prev;
			if (i == 0) {
				first_row[j] = current;
			}
			if(i!=img.rows-1){
				current->down = new Node();
			}
			if (j == 0) {
				int c_mid = 0;
				int c_down = 0;
				int c_up = 0;
				int temp = 0;
				c_mid = cost_Cmid(img, i, 0);
				c_down = c_mid + get_gray(get_coord(img, i + 1, 0));
				c_up = c_mid + get_gray(get_coord(img, i - 1, 0));
				current->energy = std::min({ c_down, c_mid, c_up });
			}
			current->col=img.at<cv::Vec3b>(i, j);
			prev = current;
			current = current->down;
		}
		prev = nullptr;
		current = current_col_start;
	}
}
void ImageGrid::print_grid() {
	Node * current_col = head;
	Node * current = head;
	int i = 0;
	int j = 0;
	while (current_col != nullptr) {
		i = 0;
		while (current != nullptr) {
			current = current->down;
			i++;
		}
		current_col = current_col->right;
		current = current_col;
		j++;
	}
}
cv::Mat ImageGrid::produce_image() {
	using namespace cv;
	Mat result = Mat(this->height,this->width, CV_8UC3);
	Node * current_col = head;
	Node * current = head;
	int i = 0;
	int j = 0;
	while (current_col != nullptr) {
		i = 0;
		while (current != nullptr) {
			result.at<Vec3b>(i, j) = current->col;
			current = current->down;
			i++;
		}
		current_col = current_col->right;
		current = current_col;
		j++;
	}
	return result;
}
void ImageGrid::resize_once() {
	Node * prev_column = head;
	Node * current_col = head->right;
	Node * current = head->right;
	Node * prev_current = head;
	int c_mid = 0;
	int c_down = 0;
	int c_up = 0;
	int temp = 0;
	int i = 0;
	int j = 0;
	while (current_col != nullptr) {
		while (current != nullptr) {
			//std::cout << ((*current).col)<<"  ";
			c_mid =  cost_Cmid_grid(*current);
			temp = get_gray(prev_current->col);
			c_down = c_mid + (current->down == nullptr ?temp: abs(temp - get_gray(current->down->col)));
			c_up = c_mid + (current->up == nullptr ?temp : abs(temp - get_gray(current->up->col)));
			if (current->up == nullptr) {
				int min = MIN((prev_current->energy) + c_mid, (prev_current->down->energy) + c_down);
				current->energy = min;
				if (min == (prev_current->energy) + c_mid) {
					current->left = prev_current;    //Central  cell
				}
				else {
					current->left = (prev_current->down); //The Bottom cell
				}
			}
			else if (current->down == nullptr) {
				int min = MIN((prev_current->energy) + c_mid, (prev_current->up->energy) + c_up);
				current->energy = min;
				if (min == (prev_current->energy) + c_mid) {
					current->left = prev_current;    //Central  cell
				}
				else {
					current->left = (prev_current->up); //The Top cell
				}
			}
			else {
				int min = MIN(MIN(prev_current->energy + c_mid,  prev_current->up->energy + c_up), prev_current->down->energy + c_down);
				current->energy = min;
				if (min == (prev_current->energy) + c_mid) {
					current->left = prev_current;    //Central  cell
				}
				else if (min == (prev_current->up->energy) + c_up) {
					current->left = (prev_current->up); //The Top cell
				}
				else {
					current->left = (prev_current->down); //The Bottom cell
				}
			}
			prev_current = prev_current->down;
			current = current->down;
		}
		current_col = current_col->right;
		prev_column = prev_column->right;
		current = current_col;
		prev_current = prev_column;
	}
	Node * min=prev_current;
	prev_current = prev_current->down;
	while (prev_current != nullptr) {
		if ((prev_current->energy) < (min->energy)) {
			min = prev_current;
		}
		prev_current = prev_current->down;
	}
	int counter = 0;
	Node * next=min->left;
	//Node *min_2 = min;
	//Node * next_2 = min->left;

	//while (min_2!= nullptr) {
	//	min_2->col = cv::Vec3b(0, 0, 255);
	//	min_2 = next_2;
	//	if (next_2 != nullptr) {
	//		next_2 = next_2->left;
	//	}
	//}
	//

	while (min!= nullptr) {
		if (min == head)std::cout << "Yes" << std::endl;
		if (min->up != nullptr) {
			(min->up)->down = min->down;
		}
		else {
			if (min->left != nullptr) {
				first_row[this->width - counter - 2]->right = min->down;
			}
			else {
				this->head = min->down;
			}
			first_row[this->width - counter - 1] =min->down;
			(min->down)->right = min->right;
		}
		if (min->down != nullptr)(min->down)->up = min->up;
		min->left = nullptr;
		delete min;
		min = next;
		if (next != nullptr) {
			next = next->left;
		}
		counter++;	
		
	}
	this->height = this->height - 1;
}
void ImageGrid::resize(int n_pixels) {
	for(int i=0;i<n_pixels;i++){
		resize_once();
	}
}