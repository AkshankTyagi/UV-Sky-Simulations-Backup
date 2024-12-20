
#include "diffuse_model.h"

/******************************************************************/
int HIP_READ_LINE(FILE *hipfile, struct STARS *line)
{
    int     i, tst;
    char    hip_line[HIP_MAIN_REC_LEN], *token, *read_res, s_tmp[13], s_tmp1[13];
    double  gl, gb;

    read_res = fgets(hip_line, HIP_MAIN_REC_LEN, hipfile);/*Read first bracket*/
    if (read_res == NULL){
        printf("%s\n", "Star not found.");
        exit(EXIT_FAILURE);
    }
    *s_tmp  = 0;
    *s_tmp1 = 0;

    token = strtok(hip_line, HIP_DELIM); /*Set up tokens*/
    for (i = 1; i < 77; ++i) {
        token = strtok(NULL, HIP_DELIM); /*read token*/
        switch (i) {
            case 1:  line->HIP_NO   = atoi(token);
                break;
            case 8:  line->ra       = atof(token);
                break;
            case 9:  line->dec      = atof(token);`
                break;
            case 11: if (atof(token) > 0)
                line->distance = 1000./atof(token);
            else line->distance = 1.e6;
                break;
            case 32: line->B_mag    = atof(token);
                break;
            case 34: line->V_mag    = atof(token);
                break;
            case 71: line->HD_NO    = atoi(token);
                break;
            case 76: strcpy(s_tmp, token);
                sscanf(s_tmp, "%s", s_tmp1);
                if (strlen(s_tmp1) == 0) strcpy(s_tmp1, "XX");
                 strcpy(line->sp_type, s_tmp1);
                break;
        }/*End switch*/
    }/*End for*/
    tst = CONV_EQ_TO_GAL(line->ra, line->dec, &gl, &gb);
    line->gl = gl;
    line->gb = gb;

    line->x = cos(DEGRAD(line->gl))*cos(DEGRAD(line->gb))*line->distance;
    line->y = sin(DEGRAD(line->gl))*cos(DEGRAD(line->gb))*line->distance;
    line->z = sin(DEGRAD(line->gb))*line->distance;

    //The Hipparcos catalog for gamma And (Almach) is wrong. It mixes the magnitude and the spctral type.
    if (line->HIP_NO == 9640) {
        line->V_mag = 5.82;
        line->B_mag = 5.02;
    }
    if (line->HIP_NO == 105259)
        strcpy(line->sp_type, "M1I");
    return(line->HIP_NO);

    return(line->HIP_NO);
}/*End READ_LINE*/
/*********************************************************************/

int EXTRA_READ_LINE(FILE *hipfile, struct STARS *line)
{
    int hip_no;
    double gl, gb, distance;
    float bmag, vmag;
    char sptype[12];


    fscanf(hipfile, "%i %lf %lf %lf %f %f %s",
           &hip_no, &gl, &gb, &distance, &bmag, &vmag, sptype);
    line->HIP_NO = hip_no;
    line->gl = gl;
    line->gb = gb;
    line->distance = distance;
    line->B_mag = bmag;
    line->V_mag = vmag;
    strcpy(line->sp_type, sptype);

    line->x = cos(DEGRAD(line->gl))*cos(DEGRAD(line->gb))*line->distance;
    line->y = sin(DEGRAD(line->gl))*cos(DEGRAD(line->gb))*line->distance;
    line->z = sin(DEGRAD(line->gb))*line->distance;

    return(line->HIP_NO);
}/*End READ_LINE*/
/*********************************************************************/

int READ_CASTELLI_SPECTRA(char *spec_dir, struct SPECTRA
                          *stellar_spectra)
{
    int status = 0, anynull;
    fitsfile *fptr;
    char filename[MAX_FILE_LENGTH];
    int i;
    int temper[N_CASTELLI_MODELS], gindex[N_CASTELLI_MODELS];
    float nullval;
    char stemper[6];

    /*Lookup table for temperatures and g*/
    for (i = 0; i <= 37; ++i)
        temper[i] = 50000 - i*1000;
    for (i = 38; i < 76; ++i)
        temper[i] = 13000 - (i - 37)*250;
    for (i = 0; i <= 4; ++i)
        gindex[i] = 12;
    for (i = 5; i <= 10; ++i)
        gindex[i] = 11;
    for (i = 11; i <= 63; ++i)
        gindex[i] = 10;
    for (i = 64; i <= 75; ++i)
        gindex[i] = 11;

    /*Read stellar spectra*/
    for (i = 0; i < N_CASTELLI_MODELS; ++i) {
        strcpy(filename, spec_dir);
        strcat(filename, "/ckp00_");
        sprintf(stemper, "%i", temper[i]);
        strcat(filename, stemper);
        strcat(filename, ".fits");
        sprintf(stellar_spectra[i].filename, "%i", temper[i]);
        fits_open_file(&fptr, filename, READONLY, &status);
        fits_movabs_hdu(fptr, 2, NULL, &status);
        fits_read_col(fptr, TFLOAT, gindex[i], 1, 1, N_CASTELLI_SPECTRA,
                      &nullval, stellar_spectra[i].spectrum, &anynull,
                      &status);
        fits_read_col(fptr, TFLOAT, 1, 1, 1, N_CASTELLI_SPECTRA,
                      &nullval, stellar_spectra[i].wavelength, &anynull,
                      &status);

        fits_close_file(fptr, &status);
    }

    return(EXIT_SUCCESS);
}

int GET_STAR_TEMP(struct STARS *hipline)
{
    char sptype[10];
    char *str=sptype;

    if (strncmp(hipline->sp_type, "sd", 2) == 0) {
        strcpy(sptype, hipline->sp_type);
        str = str + 2;
        strcpy(hipline->sp_type, str);
    }
    if (strncmp(hipline->sp_type, "O3", 2) == 0) {
        hipline->temperature = 5;
    }
    else if (strncmp(hipline->sp_type, "O4", 2) == 0) {
        hipline->temperature = 7;
    }
    else if (strncmp(hipline->sp_type, "O5", 2) == 0) {
        hipline->temperature = 10;
    }
    else if (strncmp(hipline->sp_type, "O6", 2) == 0) {
        hipline->temperature = 11;
    }
    else if (strncmp(hipline->sp_type, "O7", 2) == 0) {
        hipline->temperature = 13;
    }
    else if (strncmp(hipline->sp_type, "O8", 2) == 0) {
        hipline->temperature = 15;
    }
    else if (strncmp(hipline->sp_type, "O9", 2) == 0) {
        hipline->temperature = 18;
    }
    else if (strncmp(hipline->sp_type, "B0", 2) == 0) {
        hipline->temperature = 20;
    }
    else if (strncmp(hipline->sp_type, "B1", 2) == 0) {
        hipline->temperature = 25;
    }
    else if (strncmp(hipline->sp_type, "B2", 2) == 0) {
        hipline->temperature = 28;
    }
    else if (strncmp(hipline->sp_type, "B3", 2) == 0) {
        hipline->temperature = 31;
    }
    else if (strncmp(hipline->sp_type, "B4", 2) == 0) {
        hipline->temperature = 33;
    }
    else if (strncmp(hipline->sp_type, "B5", 2) == 0) {
        hipline->temperature = 35;
    }
    else if (strncmp(hipline->sp_type, "B6", 2) == 0) {
        hipline->temperature = 36;
    }
    else if (strncmp(hipline->sp_type, "B7", 2) == 0) {
        hipline->temperature = 37;
    }
    else if (strncmp(hipline->sp_type, "B8", 2) == 0) {
        hipline->temperature = 41;
    }
    else if (strncmp(hipline->sp_type, "B9", 2) == 0) {
        hipline->temperature = 49;
    }
    else if (strncmp(hipline->sp_type, "A0", 2) == 0) {
        hipline->temperature = 51;
    }
    else if (strncmp(hipline->sp_type, "A1", 2) == 0) {
        hipline->temperature = 52;
    }
    else if (strncmp(hipline->sp_type, "A2", 2) == 0) {
        hipline->temperature = 53;
    }
    else if (strncmp(hipline->sp_type, "A3", 2) == 0) {
        hipline->temperature = 56;
    }
    else if (strncmp(hipline->sp_type, "A4", 2) == 0) {
        hipline->temperature = 56;
    }
    else if (strncmp(hipline->sp_type, "A5", 2) == 0) {
        hipline->temperature = 56;
    }
    else if (strncmp(hipline->sp_type, "A6", 2) == 0) {
        hipline->temperature = 57;
    }
    else if (strncmp(hipline->sp_type, "A7", 2) == 0) {
        hipline->temperature = 58;
    }
    else if (strncmp(hipline->sp_type, "A8", 2) == 0) {
        hipline->temperature = 59;
    }
    else if (strncmp(hipline->sp_type, "A9", 2) == 0) {
        hipline->temperature = 60;
    }
    else if (strncmp(hipline->sp_type, "F0", 2) == 0) {
        hipline->temperature = 60;
    }
    else if (strncmp(hipline->sp_type, "F1", 2) == 0) {
        hipline->temperature = 63;
    }
    else if (strncmp(hipline->sp_type, "F2", 2) == 0) {
        hipline->temperature = 61;
    }
    else if (strncmp(hipline->sp_type, "F3", 2) == 0) {
        hipline->temperature = 62;
    }
    else if (strncmp(hipline->sp_type, "F4", 2) == 0) {
        hipline->temperature = 62;
    }
    else if (strncmp(hipline->sp_type, "F5", 2) == 0) {
        hipline->temperature = 63;
    }
    else if (strncmp(hipline->sp_type, "F6", 2) == 0) {
        hipline->temperature = 63;
    }
    else if (strncmp(hipline->sp_type, "F7", 2) == 0) {
        hipline->temperature = 63;
    }
    else if (strncmp(hipline->sp_type, "F8", 2) == 0) {
        hipline->temperature = 64;
    }
    else if (strncmp(hipline->sp_type, "F9", 2) == 0) {
        hipline->temperature = 65;
    }
    else if (strncmp(hipline->sp_type, "G0", 2) == 0) {
        hipline->temperature = 65;
    }
    else if (strncmp(hipline->sp_type, "G1", 2) == 0) {
        hipline->temperature = 66;
    }
    else if (strncmp(hipline->sp_type, "G2", 2) == 0) {
        hipline->temperature = 66;
    }
    else if (strncmp(hipline->sp_type, "G3", 2) == 0) {
        hipline->temperature = 66;
    }
    else if (strncmp(hipline->sp_type, "G4", 2) == 0) {
        hipline->temperature = 66;
    }
    else if (strncmp(hipline->sp_type, "G5", 2) == 0) {
        hipline->temperature = 66;
    }
    else if (strncmp(hipline->sp_type, "G6", 2) == 0) {
        hipline->temperature = 66;
    }
    else if (strncmp(hipline->sp_type, "G7", 2) == 0) {
        hipline->temperature = 67;
    }
    else if (strncmp(hipline->sp_type, "G8", 2) == 0) {
        hipline->temperature = 67;
    }
    else if (strncmp(hipline->sp_type, "G9", 2) == 0) {
        hipline->temperature = 67;
    }
    else if (strncmp(hipline->sp_type, "K0", 2) == 0) {
        hipline->temperature = 68;
    }
    else if (strncmp(hipline->sp_type, "K1", 2) == 0) {
        hipline->temperature = 69;
    }
    else if (strncmp(hipline->sp_type, "K2", 2) == 0) {
        hipline->temperature = 70;
    }
    else if (strncmp(hipline->sp_type, "K3", 2) == 0) {
        hipline->temperature = 70;
    }
    else if (strncmp(hipline->sp_type, "K4", 2) == 0) {
        hipline->temperature = 71;
    }
    else if (strncmp(hipline->sp_type, "K5", 2) == 0) {
        hipline->temperature = 72;
    }
    else if (strncmp(hipline->sp_type, "K6", 2) == 0) {
        hipline->temperature = 72;
    }
    else if (strncmp(hipline->sp_type, "K7", 2) == 0) {
        hipline->temperature = 73;
    }
    else if (strncmp(hipline->sp_type, "K8", 2) == 0) {
        hipline->temperature = 73;
    }
    else if (strncmp(hipline->sp_type, "K9", 2) == 0) {
        hipline->temperature = 73;
    }
    else if (strncmp(hipline->sp_type, "M0", 2) == 0) {
        hipline->temperature = 74;
    }
    else if (strncmp(hipline->sp_type, "M1", 2) == 0) {
        hipline->temperature = 74;
    }
    else if (strncmp(hipline->sp_type, "M2", 2) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "M3", 2) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "M4", 2) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "M5", 2) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "M6", 2) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "M7", 2) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "M8", 2) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "M9", 2) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "O", 1) == 0) {
        hipline->temperature = 13;
    }
    else if (strncmp(hipline->sp_type, "B", 1) == 0) {
        hipline->temperature = 35;
    }
    else if (strncmp(hipline->sp_type, "M", 1) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "C", 1) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "A", 1) == 0) {
        hipline->temperature = 56;
    }
    else if (strncmp(hipline->sp_type, "R", 1) == 0) {
        hipline->temperature = 75;
    }
    else if (strncmp(hipline->sp_type, "G", 1) == 0) {
        hipline->temperature = 66;
    }
    else if (strncmp(hipline->sp_type, "W", 1) == 0) {
        hipline->temperature = 35;
    }
    else if (strncmp(hipline->sp_type, "K", 1) == 0) {
        hipline->temperature = 72;
    }
    else if (strncmp(hipline->sp_type, "N", 1) == 0) {
        hipline->temperature = 72;
    }
    else if (strncmp(hipline->sp_type, "S", 1) == 0) {
        hipline->temperature = 72;
    }
    else if (strncmp(hipline->sp_type, "F", 1) == 0) {
        hipline->temperature = 63;
    }
    else if (strncmp(hipline->sp_type, "DA", 2) == 0) {
        hipline->temperature = 35;
    }
    /*Special Cases*/
    else {
//        printf("Unimplemented spectral type: %s\n", hipline->sp_type);
        hipline->temperature = 66;
    }
    return (EXIT_SUCCESS);
}
/**************************************************************************/

int GET_SCALE_FACTOR(struct STARS *hipstars,
                     struct SPECTRA *stellar_spectra,
                     struct INP_PAR inp_par)
{
    float scale;
    int bindex, vindex, sindex, windex = 0, doprint;
    float ebv, b_mag, v_mag, bflux, vflux, bv;
    double  tot_photon;

    if (hipstars->V_mag == 0){
        hipstars->scale = 0;/* A few stars are bad*/
        hipstars->tot_photons = 0;
    } else {
        sindex = hipstars->temperature;
        /*We scale at 4400 and 5500 Å*/
        vindex = 0;
        while (stellar_spectra[sindex].wavelength[vindex] < 5500)
            ++vindex;
        bindex = 0;
        while (stellar_spectra[sindex].wavelength[bindex] < 4400)
            ++bindex;
        while (stellar_spectra[sindex].wavelength[windex] < inp_par.wave)
            ++windex;

        bflux = stellar_spectra[sindex].spectrum[bindex];
        vflux = stellar_spectra[sindex].spectrum[vindex];
/*
 Convert the flux from energy units into magnitudes. Castelli gives the surface flux
 but I finally rescale to the flux at the Earth.
*/
        b_mag = -2.5 * log10(bflux/6.61);   //Scale for B.
        v_mag = -2.5 * log10(vflux/3.64);   //Scale for V.
        bv = hipstars->B_mag - hipstars->V_mag;
//Calculate the observed E(B-V)
        ebv = bv - (b_mag - v_mag);
        if (ebv < 0) ebv = 0;
        hipstars->ebv = ebv;
/*
 I know what the magnitude of the star is at the Earth. I take out the effects of extinction
 assuming that R is 3.1. Scale is then the flux of the star at the Earth assuming there is no
 extinction. Then I convert to the flux at a distance of 1 pc from the star.
*/
        scale = 3.64e-9*pow(10, -0.4 * (hipstars->V_mag - 3.1*ebv))/vflux;
        hipstars->scale = scale*hipstars->distance*hipstars->distance;
/*
 What I really want total number of photons from the star. Note that I really have to
 multiply by pc^2 but I'll divide by pc later so everything scales out.
*/
        tot_photon = stellar_spectra[sindex].spectrum[windex]*hipstars->scale*
            4*PI*ERG_TO_PHOT*inp_par.wave;
        hipstars->tot_photons = tot_photon;

        doprint = 0;
        if (hipstars->HD_NO == 158926)doprint = 0;
        if (hipstars->HD_NO == 160578)doprint = 0;


        if (doprint > 0){
            printf("%i %s %s %i %f %10.3e %10.3e %10.3e %10.3e\n", hipstars->HIP_NO, hipstars->sp_type,
                   stellar_spectra[sindex].filename, sindex, hipstars->V_mag, bflux, vflux,
                   hipstars->scale, stellar_spectra[sindex].spectrum[windex]);
            doprint = 0;
        }
    }

    return(EXIT_SUCCESS);
}
