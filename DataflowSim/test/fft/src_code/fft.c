#include "fft.h"

void fft(double real[FFT_SIZE], double img[FFT_SIZE], double real_twid[FFT_SIZE/2], double img_twid[FFT_SIZE/2]){
    int even, odd, span, log, rootindex;
    int real_twid_val, img_twid_val, real_odd_val, img_odd_val, real_twid_real, img_twid_img, real_twid_img, img_twid_real; 
    int real_even_val, real_odd_val, img_even_val, img_odd_val;
    int tt;
    double temp;
    log = 0;

    outer:for(span=FFT_SIZE>>1; span; span>>=1, log++){
        inner:for(odd=span; odd<FFT_SIZE; odd++){
            odd |= span;
            even = odd ^ span;

            real_even_val = real[even];
            real_odd_val = real[odd];
            img_even_val = img[even];
            img_odd_val = img[odd];

            temp = real_even_val + real_odd_val;  // Fake temp
            real_odd_val_update = real_even_val - real_odd_val;
            real_even_val_update = temp;

            temp = img_even_val + img_odd_val;  // Fake temp
            img_odd_val_update = img_even_val - img_odd_val;
            img_even_val_update = temp;

            tt = even << log;
            rootindex = tt & (FFT_SIZE - 1);
            if(rootindex){
                real_twid_val = real_twid[rootindex];
                img_twid_val = img_twid[rootindex];

                real_twid_real = real_twid_val * real_odd_val_update;
                img_twid_img = img_twid_val * img_odd_val_update;
                real_twid_img =  real_twid_val * img_odd_val_update;
                img_twid_real = img_twid_val * real_odd_val_update;

                temp = real_twid_real - img_twid_img;  // Fake temp
                img_odd_update = real_twid_img + img_twid_real;
                real_odd_update = temp;
            }
        }
    }
}
