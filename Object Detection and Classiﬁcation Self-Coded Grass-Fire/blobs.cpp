/* Applied Video Analysis of Sequences (AVSA)
 *
 *	LAB2: Blob detection & classification
 *	Lab2.0: Sample Opencv project
 *
 *
 * Authors: José M. Martínez (josem.martinez@uam.es), Paula Moral (paula.moral@uam.es), Juan C. San Miguel (juancarlos.sanmiguel@uam.es)
 */

#include "blobs.hpp"
#include <opencv2/opencv.hpp>

/**
 *	Draws blobs with different rectangles on the image 'frame'. All the input arguments must be
 *  initialized when using this function.
 *
 * \param frame Input image
 * \param pBlobList List to store the blobs found
 * \param labelled - true write label and color bb, false does not wirite label nor color bb
 *
 * \return Image containing the draw blobs. If no blobs have to be painted
 *  or arguments are wrong, the function returns a copy of the original "frame".
 *
 */
 Mat paintBlobImage(cv::Mat frame, std::vector<cvBlob> bloblist, bool labelled)
{
	cv::Mat blobImage;
	//check input conditions and return original if any is not satisfied
	//...
	frame.copyTo(blobImage);

	//required variables to paint
	//...

	//paint each blob of the list
	for(int i = 0; i < bloblist.size(); i++)
	{
		cvBlob blob = bloblist[i]; //get ith blob
		//...
		Scalar color;
		std::string label="";
		switch(blob.label){
		case PERSON:
			color = Scalar(255,0,0);
			label="PERSON";
			break;
		case CAR:
					color = Scalar(0,255,0);
					label="CAR";
					break;
		case OBJECT:
					color = Scalar(0,0,255);
					label="OBJECT";
					break;
		default:
			color = Scalar(255, 255, 255);
			label="UNKOWN";
		}

		Point p1 = Point(blob.x, blob.y);
		Point p2 = Point(blob.x+blob.w, blob.y+blob.h);

		rectangle(blobImage, p1, p2, color, 1, 8, 0);
		if (labelled)
			{
			rectangle(blobImage, p1, p2, color, 1, 8, 0);
			putText(blobImage, label, p1, FONT_HERSHEY_SIMPLEX, 0.5, color);
			}
			else
				rectangle(blobImage, p1, p2, Scalar(255, 255, 255), 1, 8, 0);
	}

	//destroy all resources (if required)
	//...

	//return the image to show
	return blobImage;
}


/**
 *	Blob extraction from 1-channel image (binary). The extraction is performed based
 *	on the analysis of the connected components. All the input arguments must be 
 *  initialized when using this function.
 *
 * \param fgmask Foreground/Background segmentation mask (1-channel binary image) 
 * \param bloblist List with found blobs
 *
 * \return Operation code (negative if not succesfull operation) 
 */

 //GLOBAL VAR
PIXEL pixel_fg;
std::vector<PIXEL> pixel_list;

int extractBlobs(cv::Mat fgmask, std::vector<cvBlob> &bloblist, int connectivity)
{	
	//check input conditions and return -1 if any is not satisfied
	//...		
	if (!fgmask.data || connectivity == NULL){
				std::cout<<"Variables are not initialized" << std::endl;
				return -1;
			}
			//required variables for connected component analysis
			//...
	            cv::Mat temp_fgmask;
	            fgmask.copyTo(temp_fgmask);

			    int counter = 0;

				Mat aux; // image to be updated each time a blob is detected (blob cleared)

				//clear blob list (to fill with this function)
				bloblist.clear();

				//Connected component analysis

				for (int x=0;x<temp_fgmask.rows;x++)
					{
						for (int y=0;y<temp_fgmask.cols;y++)
						{
							//extract connected component (blob)
							//...

							if(temp_fgmask.at<uchar>(x,y)==255)
							{
								/***   New blob (white pixel) found  ***/
								/* Save position and call neighbor analysis function */

								pixel_fg.pixel_x = x;
								pixel_fg.pixel_y = y;

								pixel_list.push_back(pixel_fg);

								cvBlob new_blob = check_nghb_pixel(connectivity, temp_fgmask);
								counter ++;
								new_blob.ID = counter;
								bloblist.push_back(new_blob);
							}
						}

					}

	// void creation of a unqie blob in the center
		//cvBlob blob=initBlob(1, fgmask.cols/2, fgmask.rows/2, fgmask.cols/4, fgmask.rows/4);
		//bloblist.push_back(blob);

//	std::cout << bkg << " " << fg << " " << sh <<" " << fill << " " << unknown << " "<< bkg+fg+sh+unknown  << " " << fgmask.rows*fgmask.cols << std::endl;
//	std::cout << blob_id << " " << small_blobs << std::endl;

	//destroy all resources
	//...

	//return OK code
	return 1;
}


int removeSmallBlobs(std::vector<cvBlob> bloblist_in, std::vector<cvBlob> &bloblist_out, int min_width, int min_height)
{
	//check input conditions and return -1 if any is not satisfied

	//required variables
	//...

	//clear blob list (to fill with this function)
	bloblist_out.clear();


	for(int i = 0; i < bloblist_in.size(); i++)
	{
		cvBlob blob_in = bloblist_in[i]; //get ith blob
		if(blob_in.w>=min_width && blob_in.h>=min_height){
		bloblist_out.push_back(blob_in);
				}

		// ...
		//bloblist_out.push_back(blob_in); // void implementation (does not remove)

	}
	//destroy all resources
	//...
	
	//return OK code
	return 1;
}



 /**
  *	Blob classification between the available classes in 'Blob.hpp' (see CLASS typedef). All the input arguments must be
  *  initialized when using this function.
  *
  * \param frame Input image
  * \param fgmask Foreground/Background segmentation mask (1-channel binary image)
  * \param bloblist List with found blobs
  *
  * \return Operation code (negative if not succesfull operation)
  */

 // ASPECT RATIO MODELS
#define MEAN_PERSON 0.3950
#define STD_PERSON 0.1887

#define MEAN_CAR 1.4736
#define STD_CAR 0.2329

#define MEAN_OBJECT 1.2111
#define STD_OBJECT 0.4470

// end ASPECT RATIO MODELS

// distances
float ED(float val1, float val2)
{
	return sqrt(pow(val1-val2,2));
}

float WED(float val1, float val2, float std)
{
	return sqrt(pow(val1-val2,2)/pow(std,2));
}
//end distances

 int classifyBlobs(std::vector<cvBlob> &bloblist)
 {
 	//required variables for classification
    float weight, height;
    float AR;
    float person, car, object;

    //classify each blob of the list
 	for(int i = 0; i < bloblist.size(); i++)
 	{
 		weight = bloblist[i].w;
 		height = bloblist[i].h;
 		AR = weight/height; //aspect ratio

 		person =  WED(AR, MEAN_PERSON, STD_PERSON);
 		car = WED(AR, MEAN_CAR, STD_CAR);
 		object = WED(AR, MEAN_OBJECT, STD_OBJECT);

 			if(person<car && person<=object){
 				bloblist[i].label = (PERSON);
 			}else if(car<person && car<=object){
 				bloblist[i].label = (CAR);
 			}else if(object<person && object<car){
 				bloblist[i].label = (OBJECT);
 			}else{
 				bloblist[i].label = (GROUP);
 			}
 			//std::cout<<bloblist[i].label << std::endl;
 	}

  	return 1;
 }

//stationary blob extraction function
 /**
  *	Stationary FG detection
  *
  * \param fgmask Foreground/Background segmentation mask (1-channel binary image)
  * \param fgmask_history Foreground history counter image (1-channel integer image)
  * \param sfgmask Foreground/Background segmentation mask (1-channel binary image)
  *
  * \return Operation code (negative if not succesfull operation)
  *
  *
  * Based on: Stationary foreground detection for video-surveillance based on foreground and motion history images, D.Ortego, J.C.SanMiguel, AVSS2013
  *
  */

#define FPS 30 //check in video - not really critical  Best so far 300 --> 0.001 We should play with values between 250 and 350
//The higher the number the longer it'll take to appear the blob on stationary FG mask
#define SECS_STATIONARY 10 // to set
#define I_COST 5 // to set // increment cost for stationarity detection wfpos
#define D_COST 5 // to set // decrement cost for stationarity detection wfneg
#define STAT_TH 0.5// to set between 0.45 and .55

 int extractStationaryFG (Mat fgmask, Mat &fgmask_history, Mat &sfgmask)
 {

	 //value used for further thresholding on equation 9
	 float numframes4static=(FPS*SECS_STATIONARY);

	 //Create a copy of fgmask not to modify its values
	 cv::Mat fgmask1 = fgmask.clone();

	 //easier to operate on floats
	 fgmask1.convertTo(fgmask1, CV_32F);
	 fgmask_history.convertTo(fgmask_history, CV_32F);

	 //getting rid of shadows when pixel values =127
 	 threshold(fgmask1,fgmask1,200,1,THRESH_BINARY);

 	 //creating the NOTForeground or Background mask
	 cv::Mat bgmask = Mat::ones(Size(fgmask1.cols,fgmask1.rows), CV_32F);
	 bgmask= bgmask - fgmask1;

	 //increase or decrease the history according to equations 2 and 3
	 fgmask_history = fgmask_history + (I_COST * fgmask1);
     fgmask_history = fgmask_history - (D_COST * bgmask);

     //TO avoid negative values in fgmask_history
 	 threshold(fgmask_history,fgmask_history,0,1024,THRESH_TOZERO);

     //update sfgmask

     	 //normalize
     	 cv::Mat fg_history_norm;
     	 fg_history_norm = min(1,fgmask_history/(numframes4static));

     	 //threshold
     	 threshold(fg_history_norm,sfgmask,STAT_TH,255,THRESH_BINARY);

     	 //blobs format
    	 sfgmask.convertTo(sfgmask, CV_8UC1);

 return 1;
 }


 PIXEL max_pix;
 PIXEL min_pix;
 cvBlob check_nghb_pixel(int connectivity, Mat temp_fgmask)
 {
	 cvBlob blob={};
	 max_pix = pixel_list.back();
	 min_pix = pixel_list.back();

	 while (!pixel_list.empty())
	 	{
		 //initalize pixels
		 pixel_fg = pixel_list.back(); // take the last element from the list
		 int x_value = pixel_fg.pixel_x; //the initial position,x for grass-fire
		 int y_value = pixel_fg.pixel_y; //the initial position,y for grass-fire
		 temp_fgmask.at<uchar>(pixel_fg.pixel_x,pixel_fg.pixel_y) = 0; // set this pixel value to 0 in foreground
		 pixel_list.pop_back(); //remove this element from the list (last one)

		 //Dealing with pixel limits
		 maxmin_coordinates();



	    //4 connectivity as default
		 if((x_value-1) > 0 ) //check x-1 pixel to the left,x
	     		{
	     			if(temp_fgmask.at<uchar>(x_value-1,y_value) == 255)
	     			{
	     				pixel_fg.pixel_x=x_value-1;
	     				pixel_fg.pixel_y=y_value;
	     				pixel_list.push_back(pixel_fg);
	     			}
	     		}

	     if((y_value-1) > 0 ) // check pixel down,y
	     		{
	     			if(temp_fgmask.at<uchar>(x_value,y_value-1) == 255)
	     			{
	     				pixel_fg.pixel_x=x_value;
	     				pixel_fg.pixel_y=y_value-1;
	     				pixel_list.push_back(pixel_fg);
	     			}
	     		}

	     if((x_value+1) < temp_fgmask.rows ) // check pixel to the right, x
	     		{
	     			if(temp_fgmask.at<uchar>(x_value+1,y_value) == 255)
	     			{
	     				pixel_fg.pixel_x=x_value+1;
	     				pixel_fg.pixel_y=y_value;
	     				pixel_list.push_back(pixel_fg);
	     			}
	     		}

	     if((y_value+1) < temp_fgmask.cols ) //check pixel above,y
	     		{
	     			if(temp_fgmask.at<uchar>(x_value,y_value+1) == 255)
	     			{
	     				pixel_fg.pixel_x=x_value;
	     				pixel_fg.pixel_y=y_value+1;
	     				pixel_list.push_back(pixel_fg);
	     			}
	     		}

	 	if(connectivity==8){ // if 8-connectivity, check pixels in diagonals
	 		if((x_value-1) >0 &&(y_value-1)>0){
	 			if(temp_fgmask.at<uchar>(x_value-1,y_value-1) == 255)
	 			    			{
	 				                    pixel_fg.pixel_x=x_value-1;
	 				    				pixel_fg.pixel_y=y_value-1;
	 				    				pixel_list.push_back(pixel_fg);
	 			    			}
	 			    			}
	 		if((x_value+1< temp_fgmask.rows) >0 &&(y_value-1)>0){
	 					if(temp_fgmask.at<uchar>(x_value+1,y_value-1) == 255)
	 					    			{
	 						                    pixel_fg.pixel_x=x_value+1;
	 						    				pixel_fg.pixel_y=y_value-1;
	 						    				pixel_list.push_back(pixel_fg);
	 					    			}
	 					    			}
	 		if((x_value-1>0) >0 &&(y_value+1)< temp_fgmask.cols){
	 							if(temp_fgmask.at<uchar>(x_value-1,y_value+1) == 255)
	 							    			{
	 								                    pixel_fg.pixel_x=x_value-1;
	 								    				pixel_fg.pixel_y=y_value+1;
	 								    				pixel_list.push_back(pixel_fg);
	 							    			}
	 							    			}
	 		if((x_value+1< temp_fgmask.rows) >0 &&(y_value+1)< temp_fgmask.cols){
	 									if(temp_fgmask.at<uchar>(x_value+1,y_value+1) == 255)
	 									    			{
	 										                    pixel_fg.pixel_x=x_value+1;
	 										    				pixel_fg.pixel_y=y_value+1;
	 										    				pixel_list.push_back(pixel_fg);
	 									    			}
	 									    			}
	 		}
	 	}

	    blob.x = (min_pix.pixel_y);
	 	blob.y = (min_pix.pixel_x);
	 	blob.h = (max_pix.pixel_x-min_pix.pixel_x); //height of the blob
	 	blob.w = (max_pix.pixel_y-min_pix.pixel_y); //weight of the blob

	 	return blob;

 }

 void maxmin_coordinates()
 {
	 		 //for right limit
	 	 	 if( pixel_fg.pixel_x > max_pix.pixel_x)
	 		 {
	 		 	max_pix.pixel_x = pixel_fg.pixel_x;
	 		 }
	 	 	 //for lower limit
	 		 if( pixel_fg.pixel_y > max_pix.pixel_y)
	 		 {
	 		 	max_pix.pixel_y = pixel_fg.pixel_y;
	 		 }
	 		 //for left limit
	 		 if( pixel_fg.pixel_x < min_pix.pixel_x)
	 		 {
	 		 	min_pix.pixel_x = pixel_fg.pixel_x;
	 		 }
	 		 //for upper limit
	 		 if( pixel_fg.pixel_y < min_pix.pixel_y)
	 		 {
	 		 min_pix.pixel_y = pixel_fg.pixel_y;
	 		 }

 }
