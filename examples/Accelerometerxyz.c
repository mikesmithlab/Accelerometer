/*****************************************************************************
* | File      	:   LCD_1in28_test.c
* | Author      :   Waveshare team
* | Function    :   1.3inch LCD  test demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-08-20
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "Accelerometerxyz.h"
#include "LCD_1in28.h"
#include "QMI8658.h"
#include <math.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"



int accelerometerxyz(void)
{
    if (DEV_Module_Init() != 0)
    {
        return -1;
    }
    
	/*Be worth checking these values for each new sensor. Start by setting one_g to 1 temporarily and then
	place on front and back. half the difference is the offset_z. Place x and y vertical. flip over and put half the difference in the offsets.
	Then set one_g to the value shown on z when it is lying flat on its back.	
	*/
	float one_g = 9.658;//This is the number V for 1g using 8g range and 8000Hz. Checked using z axis.
	float offset_x = 0.5*(9.990-9.634);
	float offset_y = 0.5*(10.796-9.011);
	float offset_z = 0.5*(10.013 - 9.303);


    printf("Accelerometer booting\r\n");
    LCD_1IN28_Init(HORIZONTAL);
    LCD_1IN28_Clear(WHITE);
    DEV_SET_PWM(60);
    // LCD_SetBacklight(1023);
    UDOUBLE Imagesize = LCD_1IN28_HEIGHT * LCD_1IN28_WIDTH * 2;
    UWORD *BlackImage;
    if ((BlackImage = (UWORD *)malloc(Imagesize)) == NULL)
    {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    Paint_NewImage((UBYTE *)BlackImage, LCD_1IN28.WIDTH, LCD_1IN28.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(WHITE);

#if 1

    float acc[3], gyro[3];
    unsigned int tim_count = 0;
    QMI8658_init();
	
	//char send_data_request;
	
    int numpts = 8000;
	
	float acc_x[numpts];
	float acc_y[numpts];
	float acc_z[numpts];
	float time_arr[numpts];
	
	float pk_acc_x;
	float pk_acc_y;
	float pk_acc_z;
	
	float av_acc_x;
	float av_acc_y;
	float av_acc_z;
	
    printf("Loaded entering main loop...\r\n");
    while (true)
    {   
		//const float conversion_factor = 3.3f / (1 << 12) * 2;
		for (int i=0; i<numpts;i++){
			QMI8658_read_acc_xyz(acc, &tim_count);	
			acc_x[i] = (acc[0] - offset_x);
			acc_y[i] = (acc[1] - offset_y);
			acc_z[i] = (acc[2] - offset_z);
			time_arr[i] = tim_count;
		}
	
		av_acc_x = 0;
		av_acc_y = 0;
		av_acc_z = 0;
		
		for (int i=0;i<numpts;i++){
			av_acc_x += acc_x[i];
			av_acc_y += acc_y[i];
			av_acc_z += acc_z[i];
		}
		av_acc_x = av_acc_x/numpts;
		av_acc_y = av_acc_y/numpts;
		av_acc_z = av_acc_z/numpts;
		
		pk_acc_x=0;
		pk_acc_y=0;
		pk_acc_z=0;
		
		for (int i=0;i<numpts;i++){
			pk_acc_x += (acc_x[i]-av_acc_x)*(acc_x[i]-av_acc_x);
			pk_acc_y += (acc_y[i]-av_acc_y)*(acc_y[i]-av_acc_y);
			pk_acc_z += (acc_z[i]-av_acc_z)*(acc_z[i]-av_acc_z);
		}
		
		pk_acc_x = sqrt(2*pk_acc_x/numpts)/one_g;
		pk_acc_y = sqrt(2*pk_acc_y/numpts)/one_g;
		pk_acc_z = sqrt(2*pk_acc_z/numpts)/one_g;
		
		av_acc_x = av_acc_x/one_g;
		av_acc_y = av_acc_y/one_g;
		av_acc_z = av_acc_z/one_g;
        
		//printf("drawing\n");
		Paint_Clear(WHITE);
		Paint_DrawString_EN(40, 44, "PK_X = ", &Font24, RED, WHITE);
        Paint_DrawString_EN(40, 71, "PK_Y = ", &Font24, GREEN, WHITE);
        Paint_DrawString_EN(40, 98, "PK_Z = ", &Font24, BLUE, WHITE);
        Paint_DrawString_EN(40, 127, "AV_X = ", &Font24, RED, WHITE);
		Paint_DrawString_EN(40, 154, "AV_Y = ", &Font24, GREEN, WHITE);
		Paint_DrawString_EN(40, 181, "AV_Z = ", &Font24, BLUE, WHITE);
        
        Paint_DrawNum(120, 44, pk_acc_x, &Font24, 2, RED, WHITE);
        Paint_DrawNum(120, 71, pk_acc_y, &Font24, 2, GREEN, WHITE);
        Paint_DrawNum(120, 98, pk_acc_z, &Font24, 2, BLUE, WHITE);
        Paint_DrawNum(120, 127, av_acc_x, &Font24, 2, RED, WHITE);
        Paint_DrawNum(120, 154, av_acc_y, &Font24, 2, GREEN, WHITE);
        Paint_DrawNum(120, 181, av_acc_z, &Font24, 2, BLUE, WHITE);

        LCD_1IN28_Display(BlackImage);
		
		//Send aggregated data
		printf("acc average x,y,z pk x,y,z (g) = %4.3f, %4.3f, %4.3f, %4.3f, %4.3f, %4.3f\r\n", av_acc_x, av_acc_y, av_acc_z, pk_acc_x, pk_acc_y, pk_acc_z);
		
		//Send raw data.
		//send_data_request = getchar();
		//if(send_data_request == 'y'){
		//	for (int i=0;i<numpts;i++){
		//printf("acc = %4.3f, %4.3f, %4.3f, %4.3f\r\n", time_arr[i], acc_x[i], acc_y[i], acc_z[i]);
		
		//	}
		//}
    }

#endif

    /* Module Exit */
    free(BlackImage);
    BlackImage = NULL;

    DEV_Module_Exit();
    return 0;
}
