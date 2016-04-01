/**
 * @file
 * This file contains the implementation of RateMatrix_LG, which is
 * class that holds a rate matrix in RevBayes.
 *
 * @brief Implementation of RateMatrix_LG
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-12-11 14:46:24 +0100 (Tue, 11 Dec 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-27, version 1.0
 * @interface Mcmc
 * @package distributions
 *
 * $Id: RateMatrix_LG.cpp 1921 2012-12-11 13:46:24Z wrightam $
 */

#include "RateMatrix_LG.h"
#include "RbException.h"
#include "RbMathMatrix.h"
#include "TransitionProbabilityMatrix.h"


using namespace RevBayesCore;

/** Construct rate matrix with n states */
RateMatrix_LG::RateMatrix_LG( void ) : RateMatrix_Empirical( 20 ){
    
    MatrixReal &m = *the_rate_matrix;
    
    /* LG */
    m[ 0][ 0] = 0.0000000; m[ 0][ 1] = 0.4250930; m[ 0][ 2] = 0.2768180; m[ 0][ 3] = 0.3951440; m[ 0][ 4] = 2.4890840;
    m[ 0][ 5] = 0.9698940; m[ 0][ 6] = 1.0385450; m[ 0][ 7] = 2.0660400; m[ 0][ 8] = 0.3588580; m[ 0][ 9] = 0.1498300;
    m[ 0][10] = 0.3953370; m[ 0][11] = 0.5365180; m[ 0][12] = 1.1240350; m[ 0][13] = 0.2537010; m[ 0][14] = 1.1776510;
    m[ 0][15] = 4.7271820; m[ 0][16] = 2.1395010; m[ 0][17] = 0.1807170; m[ 0][18] = 0.2189590; m[ 0][19] = 2.5478700;
    m[ 1][ 0] = 0.4250930; m[ 1][ 1] = 0.0000000; m[ 1][ 2] = 0.7518780; m[ 1][ 3] = 0.1239540; m[ 1][ 4] = 0.5345510;
    m[ 1][ 5] = 2.8079080; m[ 1][ 6] = 0.3639700; m[ 1][ 7] = 0.3901920; m[ 1][ 8] = 2.4266010; m[ 1][ 9] = 0.1269910;
    m[ 1][10] = 0.3018480; m[ 1][11] = 6.3260670; m[ 1][12] = 0.4841330; m[ 1][13] = 0.0527220; m[ 1][14] = 0.3325330;
    m[ 1][15] = 0.8581510; m[ 1][16] = 0.5789870; m[ 1][17] = 0.5936070; m[ 1][18] = 0.3144400; m[ 1][19] = 0.1708870;
    m[ 2][ 0] = 0.2768180; m[ 2][ 1] = 0.7518780; m[ 2][ 2] = 0.0000000; m[ 2][ 3] = 5.0761490; m[ 2][ 4] = 0.5287680;
    m[ 2][ 5] = 1.6957520; m[ 2][ 6] = 0.5417120; m[ 2][ 7] = 1.4376450; m[ 2][ 8] = 4.5092380; m[ 2][ 9] = 0.1915030;
    m[ 2][10] = 0.0684270; m[ 2][11] = 2.1450780; m[ 2][12] = 0.3710040; m[ 2][13] = 0.0895250; m[ 2][14] = 0.1617870;
    m[ 2][15] = 4.0083580; m[ 2][16] = 2.0006790; m[ 2][17] = 0.0453760; m[ 2][18] = 0.6120250; m[ 2][19] = 0.0836880;
    m[ 3][ 0] = 0.3951440; m[ 3][ 1] = 0.1239540; m[ 3][ 2] = 5.0761490; m[ 3][ 3] = 0.0000000; m[ 3][ 4] = 0.0625560;
    m[ 3][ 5] = 0.5233860; m[ 3][ 6] = 5.2438700; m[ 3][ 7] = 0.8449260; m[ 3][ 8] = 0.9271140; m[ 3][ 9] = 0.0106900;
    m[ 3][10] = 0.0150760; m[ 3][11] = 0.2829590; m[ 3][12] = 0.0255480; m[ 3][13] = 0.0174160; m[ 3][14] = 0.3944560;
    m[ 3][15] = 1.2402750; m[ 3][16] = 0.4258600; m[ 3][17] = 0.0298900; m[ 3][18] = 0.1351070; m[ 3][19] = 0.0379670;
    m[ 4][ 0] = 2.4890840; m[ 4][ 1] = 0.5345510; m[ 4][ 2] = 0.5287680; m[ 4][ 3] = 0.0625560; m[ 4][ 4] = 0.0000000;
    m[ 4][ 5] = 0.0848080; m[ 4][ 6] = 0.0034990; m[ 4][ 7] = 0.5692650; m[ 4][ 8] = 0.6405430; m[ 4][ 9] = 0.3206270;
    m[ 4][10] = 0.5940070; m[ 4][11] = 0.0132660; m[ 4][12] = 0.8936800; m[ 4][13] = 1.1052510; m[ 4][14] = 0.0753820;
    m[ 4][15] = 2.7844780; m[ 4][16] = 1.1434800; m[ 4][17] = 0.6701280; m[ 4][18] = 1.1655320; m[ 4][19] = 1.9592910;
    m[ 5][ 0] = 0.9698940; m[ 5][ 1] = 2.8079080; m[ 5][ 2] = 1.6957520; m[ 5][ 3] = 0.5233860; m[ 5][ 4] = 0.0848080;
    m[ 5][ 5] = 0.0000000; m[ 5][ 6] = 4.1285910; m[ 5][ 7] = 0.2679590; m[ 5][ 8] = 4.8135050; m[ 5][ 9] = 0.0728540;
    m[ 5][10] = 0.5824570; m[ 5][11] = 3.2342940; m[ 5][12] = 1.6725690; m[ 5][13] = 0.0358550; m[ 5][14] = 0.6242940;
    m[ 5][15] = 1.2238280; m[ 5][16] = 1.0801360; m[ 5][17] = 0.2361990; m[ 5][18] = 0.2573360; m[ 5][19] = 0.2103320;
    m[ 6][ 0] = 1.0385450; m[ 6][ 1] = 0.3639700; m[ 6][ 2] = 0.5417120; m[ 6][ 3] = 5.2438700; m[ 6][ 4] = 0.0034990;
    m[ 6][ 5] = 4.1285910; m[ 6][ 6] = 0.0000000; m[ 6][ 7] = 0.3488470; m[ 6][ 8] = 0.4238810; m[ 6][ 9] = 0.0442650;
    m[ 6][10] = 0.0696730; m[ 6][11] = 1.8071770; m[ 6][12] = 0.1737350; m[ 6][13] = 0.0188110; m[ 6][14] = 0.4194090;
    m[ 6][15] = 0.6119730; m[ 6][16] = 0.6045450; m[ 6][17] = 0.0778520; m[ 6][18] = 0.1200370; m[ 6][19] = 0.2450340;
    m[ 7][ 0] = 2.0660400; m[ 7][ 1] = 0.3901920; m[ 7][ 2] = 1.4376450; m[ 7][ 3] = 0.8449260; m[ 7][ 4] = 0.5692650;
    m[ 7][ 5] = 0.2679590; m[ 7][ 6] = 0.3488470; m[ 7][ 7] = 0.0000000; m[ 7][ 8] = 0.3114840; m[ 7][ 9] = 0.0087050;
    m[ 7][10] = 0.0442610; m[ 7][11] = 0.2966360; m[ 7][12] = 0.1395380; m[ 7][13] = 0.0895860; m[ 7][14] = 0.1969610;
    m[ 7][15] = 1.7399900; m[ 7][16] = 0.1298360; m[ 7][17] = 0.2684910; m[ 7][18] = 0.0546790; m[ 7][19] = 0.0767010;
    m[ 8][ 0] = 0.3588580; m[ 8][ 1] = 2.4266010; m[ 8][ 2] = 4.5092380; m[ 8][ 3] = 0.9271140; m[ 8][ 4] = 0.6405430;
    m[ 8][ 5] = 4.8135050; m[ 8][ 6] = 0.4238810; m[ 8][ 7] = 0.3114840; m[ 8][ 8] = 0.0000000; m[ 8][ 9] = 0.1088820;
    m[ 8][10] = 0.3663170; m[ 8][11] = 0.6972640; m[ 8][12] = 0.4424720; m[ 8][13] = 0.6821390; m[ 8][14] = 0.5088510;
    m[ 8][15] = 0.9900120; m[ 8][16] = 0.5842620; m[ 8][17] = 0.5970540; m[ 8][18] = 5.3068340; m[ 8][19] = 0.1190130;
    m[ 9][ 0] = 0.1498300; m[ 9][ 1] = 0.1269910; m[ 9][ 2] = 0.1915030; m[ 9][ 3] = 0.0106900; m[ 9][ 4] = 0.3206270;
    m[ 9][ 5] = 0.0728540; m[ 9][ 6] = 0.0442650; m[ 9][ 7] = 0.0087050; m[ 9][ 8] = 0.1088820; m[ 9][ 9] = 0.0000000;
    m[ 9][10] = 4.1450670; m[ 9][11] = 0.1590690; m[ 9][12] = 4.2736070; m[ 9][13] = 1.1127270; m[ 9][14] = 0.0782810;
    m[ 9][15] = 0.0641050; m[ 9][16] = 1.0337390; m[ 9][17] = 0.1116600; m[ 9][18] = 0.2325230; m[ 9][19] = 10.649107;
    m[10][ 0] = 0.3953370; m[10][ 1] = 0.3018480; m[10][ 2] = 0.0684270; m[10][ 3] = 0.0150760; m[10][ 4] = 0.5940070;
    m[10][ 5] = 0.5824570; m[10][ 6] = 0.0696730; m[10][ 7] = 0.0442610; m[10][ 8] = 0.3663170; m[10][ 9] = 4.1450670;
    m[10][10] = 0.0000000; m[10][11] = 0.1375000; m[10][12] = 6.3123580; m[10][13] = 2.5926920; m[10][14] = 0.2490600;
    m[10][15] = 0.1822870; m[10][16] = 0.3029360; m[10][17] = 0.6196320; m[10][18] = 0.2996480; m[10][19] = 1.7027450;
    m[11][ 0] = 0.5365180; m[11][ 1] = 6.3260670; m[11][ 2] = 2.1450780; m[11][ 3] = 0.2829590; m[11][ 4] = 0.0132660;
    m[11][ 5] = 3.2342940; m[11][ 6] = 1.8071770; m[11][ 7] = 0.2966360; m[11][ 8] = 0.6972640; m[11][ 9] = 0.1590690;
    m[11][10] = 0.1375000; m[11][11] = 0.0000000; m[11][12] = 0.6566040; m[11][13] = 0.0239180; m[11][14] = 0.3903220;
    m[11][15] = 0.7486830; m[11][16] = 1.1368630; m[11][17] = 0.0499060; m[11][18] = 0.1319320; m[11][19] = 0.1852020;
    m[12][ 0] = 1.1240350; m[12][ 1] = 0.4841330; m[12][ 2] = 0.3710040; m[12][ 3] = 0.0255480; m[12][ 4] = 0.8936800;
    m[12][ 5] = 1.6725690; m[12][ 6] = 0.1737350; m[12][ 7] = 0.1395380; m[12][ 8] = 0.4424720; m[12][ 9] = 4.2736070;
    m[12][10] = 6.3123580; m[12][11] = 0.6566040; m[12][12] = 0.0000000; m[12][13] = 1.7988530; m[12][14] = 0.0998490;
    m[12][15] = 0.3469600; m[12][16] = 2.0203660; m[12][17] = 0.6961750; m[12][18] = 0.4813060; m[12][19] = 1.8987180;
    m[13][ 0] = 0.2537010; m[13][ 1] = 0.0527220; m[13][ 2] = 0.0895250; m[13][ 3] = 0.0174160; m[13][ 4] = 1.1052510;
    m[13][ 5] = 0.0358550; m[13][ 6] = 0.0188110; m[13][ 7] = 0.0895860; m[13][ 8] = 0.6821390; m[13][ 9] = 1.1127270;
    m[13][10] = 2.5926920; m[13][11] = 0.0239180; m[13][12] = 1.7988530; m[13][13] = 0.0000000; m[13][14] = 0.0944640;
    m[13][15] = 0.3618190; m[13][16] = 0.1650010; m[13][17] = 2.4571210; m[13][18] = 7.8039020; m[13][19] = 0.6546830;
    m[14][ 0] = 1.1776510; m[14][ 1] = 0.3325330; m[14][ 2] = 0.1617870; m[14][ 3] = 0.3944560; m[14][ 4] = 0.0753820;
    m[14][ 5] = 0.6242940; m[14][ 6] = 0.4194090; m[14][ 7] = 0.1969610; m[14][ 8] = 0.5088510; m[14][ 9] = 0.0782810;
    m[14][10] = 0.2490600; m[14][11] = 0.3903220; m[14][12] = 0.0998490; m[14][13] = 0.0944640; m[14][14] = 0.0000000;
    m[14][15] = 1.3381320; m[14][16] = 0.5714680; m[14][17] = 0.0951310; m[14][18] = 0.0896130; m[14][19] = 0.2965010;
    m[15][ 0] = 4.7271820; m[15][ 1] = 0.8581510; m[15][ 2] = 4.0083580; m[15][ 3] = 1.2402750; m[15][ 4] = 2.7844780;
    m[15][ 5] = 1.2238280; m[15][ 6] = 0.6119730; m[15][ 7] = 1.7399900; m[15][ 8] = 0.9900120; m[15][ 9] = 0.0641050;
    m[15][10] = 0.1822870; m[15][11] = 0.7486830; m[15][12] = 0.3469600; m[15][13] = 0.3618190; m[15][14] = 1.3381320;
    m[15][15] = 0.0000000; m[15][16] = 6.4722790; m[15][17] = 0.2488620; m[15][18] = 0.4005470; m[15][19] = 0.0983690;
    m[16][ 0] = 2.1395010; m[16][ 1] = 0.5789870; m[16][ 2] = 2.0006790; m[16][ 3] = 0.4258600; m[16][ 4] = 1.1434800;
    m[16][ 5] = 1.0801360; m[16][ 6] = 0.6045450; m[16][ 7] = 0.1298360; m[16][ 8] = 0.5842620; m[16][ 9] = 1.0337390;
    m[16][10] = 0.3029360; m[16][11] = 1.1368630; m[16][12] = 2.0203660; m[16][13] = 0.1650010; m[16][14] = 0.5714680;
    m[16][15] = 6.4722790; m[16][16] = 0.0000000; m[16][17] = 0.1408250; m[16][18] = 0.2458410; m[16][19] = 2.1881580;
    m[17][ 0] = 0.1807170; m[17][ 1] = 0.5936070; m[17][ 2] = 0.0453760; m[17][ 3] = 0.0298900; m[17][ 4] = 0.6701280;
    m[17][ 5] = 0.2361990; m[17][ 6] = 0.0778520; m[17][ 7] = 0.2684910; m[17][ 8] = 0.5970540; m[17][ 9] = 0.1116600;
    m[17][10] = 0.6196320; m[17][11] = 0.0499060; m[17][12] = 0.6961750; m[17][13] = 2.4571210; m[17][14] = 0.0951310;
    m[17][15] = 0.2488620; m[17][16] = 0.1408250; m[17][17] = 0.0000000; m[17][18] = 3.1518150; m[17][19] = 0.1895100;
    m[18][ 0] = 0.2189590; m[18][ 1] = 0.3144400; m[18][ 2] = 0.6120250; m[18][ 3] = 0.1351070; m[18][ 4] = 1.1655320;
    m[18][ 5] = 0.2573360; m[18][ 6] = 0.1200370; m[18][ 7] = 0.0546790; m[18][ 8] = 5.3068340; m[18][ 9] = 0.2325230;
    m[18][10] = 0.2996480; m[18][11] = 0.1319320; m[18][12] = 0.4813060; m[18][13] = 7.8039020; m[18][14] = 0.0896130;
    m[18][15] = 0.4005470; m[18][16] = 0.2458410; m[18][17] = 3.1518150; m[18][18] = 0.0000000; m[18][19] = 0.2493130;
    m[19][ 0] = 2.5478700; m[19][ 1] = 0.1708870; m[19][ 2] = 0.0836880; m[19][ 3] = 0.0379670; m[19][ 4] = 1.9592910;
    m[19][ 5] = 0.2103320; m[19][ 6] = 0.2450340; m[19][ 7] = 0.0767010; m[19][ 8] = 0.1190130; m[19][ 9] = 10.649107;
    m[19][10] = 1.7027450; m[19][11] = 0.1852020; m[19][12] = 1.8987180; m[19][13] = 0.6546830; m[19][14] = 0.2965010;
    m[19][15] = 0.0983690; m[19][16] = 2.1881580; m[19][17] = 0.1895100; m[19][18] = 0.2493130; m[19][19] = 0.0000000;
    
    stationary_freqs[ 0] = 0.079066;
    stationary_freqs[ 1] = 0.055941;
    stationary_freqs[ 2] = 0.041977;
    stationary_freqs[ 3] = 0.053052;
    stationary_freqs[ 4] = 0.012937;
    stationary_freqs[ 5] = 0.040767;
    stationary_freqs[ 6] = 0.071586;
    stationary_freqs[ 7] = 0.057337;
    stationary_freqs[ 8] = 0.022355;
    stationary_freqs[ 9] = 0.062157;
    stationary_freqs[10] = 0.099081;
    stationary_freqs[11] = 0.064600;
    stationary_freqs[12] = 0.022951;
    stationary_freqs[13] = 0.042302;
    stationary_freqs[14] = 0.044040;
    stationary_freqs[15] = 0.061197;
    stationary_freqs[16] = 0.053287;
    stationary_freqs[17] = 0.012066;
    stationary_freqs[18] = 0.034155;
    stationary_freqs[19] = 0.069147;

    // set the diagonal values
    setDiagonal();
    
    // rescale 
    rescaleToAverageRate( 1.0 );
    
    // update the eigensystem
    updateEigenSystem();
    
}


/** Destructor */
RateMatrix_LG::~RateMatrix_LG(void) {
    
}




RateMatrix_LG* RateMatrix_LG::clone( void ) const {
    return new RateMatrix_LG( *this );
}

