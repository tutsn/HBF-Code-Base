/* Gamma correction table until pow() is fixed 
const uint8_t GAMMA_2811[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
    6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
    11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
    19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
    29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
    40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
    71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
    90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
    111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
    135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
    162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
    191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
    222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255
};*/

/* More "aggressive" gamma correction based on CIE 1931 */
const uint8_t GAMMA_2811[] = {
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 
  4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 
  7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 
  11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 
  17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 
  25, 26, 26, 27, 28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 34, 34, 
  35, 36, 37, 37, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 
  47, 48, 49, 50, 51, 52, 53, 54, 54, 55, 56, 57, 58, 59, 60, 61, 
  62, 63, 64, 65, 66, 67, 68, 70, 71, 72, 73, 74, 75, 76, 77, 79, 
  80, 81, 82, 83, 85, 86, 87, 88, 90, 91, 92, 94, 95, 96, 98, 99, 
  100,102,103,105,106,108,109,110,112,113,115,116,118,120,121,123,
  124,126,128,129,131,132,134,136,138,139,141,143,145,146,148,150,
  152,154,155,157,159,161,163,165,167,169,171,173,175,177,179,181,
  183,185,187,189,191,193,196,198,200,202,204,207,209,211,214,216,
  218,220,223,225,228,230,232,235,237,240,242,245,247,250,252,255
};

/* 8bit -> 12bit PWN Gamma correction for single channel dmx values - based on CIE 1931 */
const uint16_t GAMMA_PCA8[] = {
  0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 18, 20, 21, 23, 25, 27, 
  28, 30, 32, 34, 36, 37, 39, 41, 43, 45, 47, 49, 52, 54, 56, 59, 
  61, 64, 66, 69, 72, 75, 77, 80, 83, 87, 90, 93, 96, 100, 103, 107, 
  111, 115, 118, 122, 126, 131, 135, 139, 144, 148, 153, 157, 162, 167, 172, 177, 
  182, 187, 193, 198, 204, 209, 215, 221, 227, 233, 239, 246, 252, 259, 265, 272, 
  279, 286, 293, 300, 308, 315, 323, 330, 338, 346, 354, 362, 371, 379, 388, 396, 
  405, 414, 423, 432, 442, 451, 461, 470, 480, 490, 501, 511, 521, 532, 543, 553, 
  564, 576, 587, 598, 610, 622, 634, 646, 658, 670, 683, 695, 708, 721, 734, 748, 
  761, 775, 788, 802, 816, 831, 845, 860, 874, 889, 904, 920, 935, 951, 966, 982, 
  999,1015,1031,1048,1065,1082,1099,1116,1134,1152,1170,1188,1206,1224,1243,1262,
  1281,1300,1320,1339,1359,1379,1399,1420,1440,1461,1482,1503,1525,1546,1568,1590,
  1612,1635,1657,1680,1703,1726,1750,1774,1797,1822,1846,1870,1895,1920,1945,1971,
  1996,2022,2048,2074,2101,2128,2155,2182,2209,2237,2265,2293,2321,2350,2378,2407,
  2437,2466,2496,2526,2556,2587,2617,2648,2679,2711,2743,2774,2807,2839,2872,2905,
  2938,2971,3005,3039,3073,3107,3142,3177,3212,3248,3283,3319,3356,3392,3429,3466,
  3503,3541,3578,3617,3655,3694,3732,3772,3811,3851,3891,3931,3972,4012,4054,4095
};
  
/* 12bit PWN Gamma correction for 2 channel/16bit dmx values divided by 4 - based on CIE 1931 */
const uint16_t GAMMA_PCA12[] = {
  0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 
  5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 
  9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 
  11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 
  12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 
  14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 
  16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 
  18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 
  20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 
  21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 
  23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 
  25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 
  27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 
  30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 
  32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 
  34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 
  35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 
  37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 
  39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 
  41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 
  43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 
  45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 
  47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 
  49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 
  51, 51, 52, 52, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 
  54, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 56, 56, 
  56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 
  58, 58, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 61, 
  61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 
  63, 63, 64, 64, 64, 64, 64, 64, 65, 65, 65, 65, 65, 65, 66, 66, 
  66, 66, 66, 66, 67, 67, 67, 67, 67, 67, 68, 68, 68, 68, 68, 68, 
  69, 69, 69, 69, 69, 69, 70, 70, 70, 70, 70, 70, 71, 71, 71, 71, 
  71, 71, 72, 72, 72, 72, 72, 73, 73, 73, 73, 73, 73, 74, 74, 74, 
  74, 74, 74, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 76, 77, 77, 
  77, 77, 77, 78, 78, 78, 78, 78, 78, 79, 79, 79, 79, 79, 80, 80, 
  80, 80, 80, 81, 81, 81, 81, 81, 81, 82, 82, 82, 82, 82, 83, 83, 
  83, 83, 83, 84, 84, 84, 84, 84, 85, 85, 85, 85, 85, 86, 86, 86, 
  86, 86, 87, 87, 87, 87, 87, 88, 88, 88, 88, 88, 89, 89, 89, 89, 
  89, 90, 90, 90, 90, 90, 91, 91, 91, 91, 91, 92, 92, 92, 92, 92, 
  93, 93, 93, 93, 93, 94, 94, 94, 94, 94, 95, 95, 95, 95, 96, 96, 
  96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 
  99, 100, 100, 100, 100, 100, 101, 101, 101, 101, 102, 102, 102, 102, 102, 103, 
  103, 103, 103, 104, 104, 104, 104, 104, 105, 105, 105, 105, 106, 106, 106, 106, 
  106, 107, 107, 107, 107, 108, 108, 108, 108, 109, 109, 109, 109, 109, 110, 110, 
  110, 110, 111, 111, 111, 111, 112, 112, 112, 112, 112, 113, 113, 113, 113, 114, 
  114, 114, 114, 115, 115, 115, 115, 116, 116, 116, 116, 117, 117, 117, 117, 117, 
  118, 118, 118, 118, 119, 119, 119, 119, 120, 120, 120, 120, 121, 121, 121, 121, 
  122, 122, 122, 122, 123, 123, 123, 123, 124, 124, 124, 124, 125, 125, 125, 125, 
  126, 126, 126, 126, 127, 127, 127, 127, 128, 128, 128, 128, 129, 129, 129, 130, 
  130, 130, 130, 131, 131, 131, 131, 132, 132, 132, 132, 133, 133, 133, 133, 134, 
  134, 134, 134, 135, 135, 135, 136, 136, 136, 136, 137, 137, 137, 137, 138, 138, 
  138, 139, 139, 139, 139, 140, 140, 140, 140, 141, 141, 141, 142, 142, 142, 142, 
  143, 143, 143, 143, 144, 144, 144, 145, 145, 145, 145, 146, 146, 146, 146, 147, 
  147, 147, 148, 148, 148, 148, 149, 149, 149, 150, 150, 150, 150, 151, 151, 151, 
  152, 152, 152, 152, 153, 153, 153, 154, 154, 154, 155, 155, 155, 155, 156, 156, 
  156, 157, 157, 157, 157, 158, 158, 158, 159, 159, 159, 159, 160, 160, 160, 161, 
  161, 161, 162, 162, 162, 162, 163, 163, 163, 164, 164, 164, 165, 165, 165, 166, 
  166, 166, 166, 167, 167, 167, 168, 168, 168, 169, 169, 169, 170, 170, 170, 170, 
  171, 171, 171, 172, 172, 172, 173, 173, 173, 174, 174, 174, 175, 175, 175, 175, 
  176, 176, 176, 177, 177, 177, 178, 178, 178, 179, 179, 179, 180, 180, 180, 181, 
  181, 181, 182, 182, 182, 183, 183, 183, 183, 184, 184, 184, 185, 185, 185, 186, 
  186, 186, 187, 187, 187, 188, 188, 188, 189, 189, 189, 190, 190, 190, 191, 191, 
  191, 192, 192, 192, 193, 193, 193, 194, 194, 194, 195, 195, 195, 196, 196, 197, 
  197, 197, 198, 198, 198, 199, 199, 199, 200, 200, 200, 201, 201, 201, 202, 202, 
  202, 203, 203, 203, 204, 204, 204, 205, 205, 206, 206, 206, 207, 207, 207, 208, 
  208, 208, 209, 209, 209, 210, 210, 210, 211, 211, 212, 212, 212, 213, 213, 213, 
  214, 214, 214, 215, 215, 216, 216, 216, 217, 217, 217, 218, 218, 218, 219, 219, 
  220, 220, 220, 221, 221, 221, 222, 222, 223, 223, 223, 224, 224, 224, 225, 225, 
  225, 226, 226, 227, 227, 227, 228, 228, 229, 229, 229, 230, 230, 230, 231, 231, 
  232, 232, 232, 233, 233, 233, 234, 234, 235, 235, 235, 236, 236, 237, 237, 237, 
  238, 238, 238, 239, 239, 240, 240, 240, 241, 241, 242, 242, 242, 243, 243, 244, 
  244, 244, 245, 245, 246, 246, 246, 247, 247, 248, 248, 248, 249, 249, 250, 250, 
  250, 251, 251, 252, 252, 252, 253, 253, 254, 254, 254, 255, 255, 256, 256, 256, 
  257, 257, 258, 258, 258, 259, 259, 260, 260, 260, 261, 261, 262, 262, 263, 263, 
  263, 264, 264, 265, 265, 265, 266, 266, 267, 267, 268, 268, 268, 269, 269, 270, 
  270, 270, 271, 271, 272, 272, 273, 273, 273, 274, 274, 275, 275, 276, 276, 276, 
  277, 277, 278, 278, 279, 279, 279, 280, 280, 281, 281, 282, 282, 283, 283, 283, 
  284, 284, 285, 285, 286, 286, 286, 287, 287, 288, 288, 289, 289, 290, 290, 290, 
  291, 291, 292, 292, 293, 293, 294, 294, 294, 295, 295, 296, 296, 297, 297, 298, 
  298, 298, 299, 299, 300, 300, 301, 301, 302, 302, 303, 303, 303, 304, 304, 305, 
  305, 306, 306, 307, 307, 308, 308, 308, 309, 309, 310, 310, 311, 311, 312, 312, 
  313, 313, 314, 314, 315, 315, 315, 316, 316, 317, 317, 318, 318, 319, 319, 320, 
  320, 321, 321, 322, 322, 323, 323, 323, 324, 324, 325, 325, 326, 326, 327, 327, 
  328, 328, 329, 329, 330, 330, 331, 331, 332, 332, 333, 333, 334, 334, 335, 335, 
  336, 336, 337, 337, 337, 338, 338, 339, 339, 340, 340, 341, 341, 342, 342, 343, 
  343, 344, 344, 345, 345, 346, 346, 347, 347, 348, 348, 349, 349, 350, 350, 351, 
  351, 352, 352, 353, 353, 354, 354, 355, 355, 356, 356, 357, 357, 358, 358, 359, 
  360, 360, 361, 361, 362, 362, 363, 363, 364, 364, 365, 365, 366, 366, 367, 367, 
  368, 368, 369, 369, 370, 370, 371, 371, 372, 372, 373, 373, 374, 375, 375, 376, 
  376, 377, 377, 378, 378, 379, 379, 380, 380, 381, 381, 382, 382, 383, 384, 384, 
  385, 385, 386, 386, 387, 387, 388, 388, 389, 389, 390, 390, 391, 392, 392, 393, 
  393, 394, 394, 395, 395, 396, 396, 397, 398, 398, 399, 399, 400, 400, 401, 401, 
  402, 402, 403, 404, 404, 405, 405, 406, 406, 407, 407, 408, 409, 409, 410, 410, 
  411, 411, 412, 412, 413, 414, 414, 415, 415, 416, 416, 417, 418, 418, 419, 419, 
  420, 420, 421, 422, 422, 423, 423, 424, 424, 425, 426, 426, 427, 427, 428, 428, 
  429, 430, 430, 431, 431, 432, 432, 433, 434, 434, 435, 435, 436, 436, 437, 438, 
  438, 439, 439, 440, 441, 441, 442, 442, 443, 443, 444, 445, 445, 446, 446, 447, 
  448, 448, 449, 449, 450, 451, 451, 452, 452, 453, 454, 454, 455, 455, 456, 457, 
  457, 458, 458, 459, 460, 460, 461, 461, 462, 463, 463, 464, 464, 465, 466, 466, 
  467, 467, 468, 469, 469, 470, 470, 471, 472, 472, 473, 474, 474, 475, 475, 476, 
  477, 477, 478, 478, 479, 480, 480, 481, 482, 482, 483, 483, 484, 485, 485, 486, 
  487, 487, 488, 488, 489, 490, 490, 491, 492, 492, 493, 493, 494, 495, 495, 496, 
  497, 497, 498, 498, 499, 500, 500, 501, 502, 502, 503, 504, 504, 505, 506, 506, 
  507, 507, 508, 509, 509, 510, 511, 511, 512, 513, 513, 514, 515, 515, 516, 516, 
  517, 518, 518, 519, 520, 520, 521, 522, 522, 523, 524, 524, 525, 526, 526, 527, 
  528, 528, 529, 530, 530, 531, 532, 532, 533, 534, 534, 535, 536, 536, 537, 538, 
  538, 539, 540, 540, 541, 542, 542, 543, 544, 544, 545, 546, 546, 547, 548, 548, 
  549, 550, 550, 551, 552, 552, 553, 554, 554, 555, 556, 556, 557, 558, 559, 559, 
  560, 561, 561, 562, 563, 563, 564, 565, 565, 566, 567, 568, 568, 569, 570, 570, 
  571, 572, 572, 573, 574, 574, 575, 576, 577, 577, 578, 579, 579, 580, 581, 581, 
  582, 583, 584, 584, 585, 586, 586, 587, 588, 589, 589, 590, 591, 591, 592, 593, 
  594, 594, 595, 596, 596, 597, 598, 599, 599, 600, 601, 601, 602, 603, 604, 604, 
  605, 606, 606, 607, 608, 609, 609, 610, 611, 612, 612, 613, 614, 614, 615, 616, 
  617, 617, 618, 619, 620, 620, 621, 622, 623, 623, 624, 625, 625, 626, 627, 628, 
  628, 629, 630, 631, 631, 632, 633, 634, 634, 635, 636, 637, 637, 638, 639, 640, 
  640, 641, 642, 643, 643, 644, 645, 646, 646, 647, 648, 649, 649, 650, 651, 652, 
  652, 653, 654, 655, 656, 656, 657, 658, 659, 659, 660, 661, 662, 662, 663, 664, 
  665, 665, 666, 667, 668, 669, 669, 670, 671, 672, 672, 673, 674, 675, 676, 676, 
  677, 678, 679, 679, 680, 681, 682, 683, 683, 684, 685, 686, 686, 687, 688, 689, 
  690, 690, 691, 692, 693, 694, 694, 695, 696, 697, 698, 698, 699, 700, 701, 702, 
  702, 703, 704, 705, 706, 706, 707, 708, 709, 710, 710, 711, 712, 713, 714, 714, 
  715, 716, 717, 718, 718, 719, 720, 721, 722, 722, 723, 724, 725, 726, 727, 727, 
  728, 729, 730, 731, 731, 732, 733, 734, 735, 736, 736, 737, 738, 739, 740, 741, 
  741, 742, 743, 744, 745, 745, 746, 747, 748, 749, 750, 750, 751, 752, 753, 754, 
  755, 756, 756, 757, 758, 759, 760, 761, 761, 762, 763, 764, 765, 766, 766, 767, 
  768, 769, 770, 771, 772, 772, 773, 774, 775, 776, 777, 778, 778, 779, 780, 781, 
  782, 783, 783, 784, 785, 786, 787, 788, 789, 790, 790, 791, 792, 793, 794, 795, 
  796, 796, 797, 798, 799, 800, 801, 802, 803, 803, 804, 805, 806, 807, 808, 809, 
  810, 810, 811, 812, 813, 814, 815, 816, 817, 817, 818, 819, 820, 821, 822, 823, 
  824, 825, 825, 826, 827, 828, 829, 830, 831, 832, 833, 833, 834, 835, 836, 837, 
  838, 839, 840, 841, 842, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 851, 
  852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 862, 863, 864, 865, 866, 
  867, 868, 869, 870, 871, 872, 873, 873, 874, 875, 876, 877, 878, 879, 880, 881, 
  882, 883, 884, 885, 886, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 
  897, 898, 899, 900, 901, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 
  912, 913, 914, 915, 916, 917, 918, 919, 919, 920, 921, 922, 923, 924, 925, 926, 
  927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 942, 
  943, 944, 945, 946, 947, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 
  958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 
  974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 
  990, 991, 992, 993, 994, 995, 996, 997, 998, 999,1000,1001,1002,1003,1004, 1005, 
  1006,1007,1008,1009,1010,1011,1012,1013,1014,1015,1016,1017,1018,1019,1020,1021,
  1022,1024,1025,1026,1027,1028,1029,1030,1031,1032,1033,1034,1035,1036,1037,1038,
  1039,1040,1041,1042,1043,1044,1045,1046,1047,1048,1049,1050,1051,1053,1054,1055,
  1056,1057,1058,1059,1060,1061,1062,1063,1064,1065,1066,1067,1068,1069,1070,1071,
  1072,1074,1075,1076,1077,1078,1079,1080,1081,1082,1083,1084,1085,1086,1087,1088,
  1090,1091,1092,1093,1094,1095,1096,1097,1098,1099,1100,1101,1102,1103,1105,1106,
  1107,1108,1109,1110,1111,1112,1113,1114,1115,1116,1118,1119,1120,1121,1122,1123,
  1124,1125,1126,1127,1128,1130,1131,1132,1133,1134,1135,1136,1137,1138,1139,1141,
  1142,1143,1144,1145,1146,1147,1148,1149,1151,1152,1153,1154,1155,1156,1157,1158,
  1159,1161,1162,1163,1164,1165,1166,1167,1168,1170,1171,1172,1173,1174,1175,1176,
  1177,1178,1180,1181,1182,1183,1184,1185,1186,1188,1189,1190,1191,1192,1193,1194,
  1195,1197,1198,1199,1200,1201,1202,1203,1205,1206,1207,1208,1209,1210,1211,1213,
  1214,1215,1216,1217,1218,1220,1221,1222,1223,1224,1225,1226,1228,1229,1230,1231,
  1232,1233,1235,1236,1237,1238,1239,1240,1242,1243,1244,1245,1246,1247,1249,1250,
  1251,1252,1253,1254,1256,1257,1258,1259,1260,1262,1263,1264,1265,1266,1267,1269,
  1270,1271,1272,1273,1275,1276,1277,1278,1279,1281,1282,1283,1284,1285,1286,1288,
  1289,1290,1291,1292,1294,1295,1296,1297,1298,1300,1301,1302,1303,1304,1306,1307,
  1308,1309,1311,1312,1313,1314,1315,1317,1318,1319,1320,1321,1323,1324,1325,1326,
  1328,1329,1330,1331,1332,1334,1335,1336,1337,1339,1340,1341,1342,1343,1345,1346,
  1347,1348,1350,1351,1352,1353,1355,1356,1357,1358,1360,1361,1362,1363,1364,1366,
  1367,1368,1369,1371,1372,1373,1374,1376,1377,1378,1379,1381,1382,1383,1384,1386,
  1387,1388,1390,1391,1392,1393,1395,1396,1397,1398,1400,1401,1402,1403,1405,1406,
  1407,1408,1410,1411,1412,1414,1415,1416,1417,1419,1420,1421,1422,1424,1425,1426,
  1428,1429,1430,1431,1433,1434,1435,1437,1438,1439,1440,1442,1443,1444,1446,1447,
  1448,1449,1451,1452,1453,1455,1456,1457,1459,1460,1461,1462,1464,1465,1466,1468,
  1469,1470,1472,1473,1474,1476,1477,1478,1479,1481,1482,1483,1485,1486,1487,1489,
  1490,1491,1493,1494,1495,1497,1498,1499,1501,1502,1503,1505,1506,1507,1509,1510,
  1511,1512,1514,1515,1516,1518,1519,1520,1522,1523,1525,1526,1527,1529,1530,1531,
  1533,1534,1535,1537,1538,1539,1541,1542,1543,1545,1546,1547,1549,1550,1551,1553,
  1554,1555,1557,1558,1560,1561,1562,1564,1565,1566,1568,1569,1570,1572,1573,1575,
  1576,1577,1579,1580,1581,1583,1584,1586,1587,1588,1590,1591,1592,1594,1595,1597,
  1598,1599,1601,1602,1603,1605,1606,1608,1609,1610,1612,1613,1615,1616,1617,1619,
  1620,1622,1623,1624,1626,1627,1629,1630,1631,1633,1634,1636,1637,1638,1640,1641,
  1643,1644,1645,1647,1648,1650,1651,1652,1654,1655,1657,1658,1659,1661,1662,1664,
  1665,1667,1668,1669,1671,1672,1674,1675,1677,1678,1679,1681,1682,1684,1685,1687,
  1688,1689,1691,1692,1694,1695,1697,1698,1699,1701,1702,1704,1705,1707,1708,1710,
  1711,1712,1714,1715,1717,1718,1720,1721,1723,1724,1725,1727,1728,1730,1731,1733,
  1734,1736,1737,1739,1740,1742,1743,1744,1746,1747,1749,1750,1752,1753,1755,1756,
  1758,1759,1761,1762,1764,1765,1766,1768,1769,1771,1772,1774,1775,1777,1778,1780,
  1781,1783,1784,1786,1787,1789,1790,1792,1793,1795,1796,1798,1799,1801,1802,1804,
  1805,1807,1808,1810,1811,1813,1814,1816,1817,1819,1820,1822,1823,1825,1826,1828,
  1829,1831,1832,1834,1835,1837,1838,1840,1841,1843,1844,1846,1847,1849,1850,1852,
  1854,1855,1857,1858,1860,1861,1863,1864,1866,1867,1869,1870,1872,1873,1875,1876,
  1878,1880,1881,1883,1884,1886,1887,1889,1890,1892,1893,1895,1897,1898,1900,1901,
  1903,1904,1906,1907,1909,1911,1912,1914,1915,1917,1918,1920,1921,1923,1925,1926,
  1928,1929,1931,1932,1934,1936,1937,1939,1940,1942,1943,1945,1947,1948,1950,1951,
  1953,1954,1956,1958,1959,1961,1962,1964,1965,1967,1969,1970,1972,1973,1975,1977,
  1978,1980,1981,1983,1985,1986,1988,1989,1991,1993,1994,1996,1997,1999,2001,2002,
  2004,2005,2007,2009,2010,2012,2013,2015,2017,2018,2020,2021,2023,2025,2026,2028,
  2030,2031,2033,2034,2036,2038,2039,2041,2043,2044,2046,2047,2049,2051,2052,2054,
  2056,2057,2059,2061,2062,2064,2065,2067,2069,2070,2072,2074,2075,2077,2079,2080,
  2082,2083,2085,2087,2088,2090,2092,2093,2095,2097,2098,2100,2102,2103,2105,2107,
  2108,2110,2112,2113,2115,2117,2118,2120,2122,2123,2125,2127,2128,2130,2132,2133,
  2135,2137,2138,2140,2142,2143,2145,2147,2148,2150,2152,2153,2155,2157,2159,2160,
  2162,2164,2165,2167,2169,2170,2172,2174,2175,2177,2179,2181,2182,2184,2186,2187,
  2189,2191,2192,2194,2196,2198,2199,2201,2203,2204,2206,2208,2210,2211,2213,2215,
  2216,2218,2220,2222,2223,2225,2227,2228,2230,2232,2234,2235,2237,2239,2241,2242,
  2244,2246,2247,2249,2251,2253,2254,2256,2258,2260,2261,2263,2265,2267,2268,2270,
  2272,2274,2275,2277,2279,2281,2282,2284,2286,2288,2289,2291,2293,2295,2296,2298,
  2300,2302,2303,2305,2307,2309,2310,2312,2314,2316,2318,2319,2321,2323,2325,2326,
  2328,2330,2332,2334,2335,2337,2339,2341,2342,2344,2346,2348,2350,2351,2353,2355,
  2357,2358,2360,2362,2364,2366,2367,2369,2371,2373,2375,2376,2378,2380,2382,2384,
  2385,2387,2389,2391,2393,2394,2396,2398,2400,2402,2404,2405,2407,2409,2411,2413,
  2414,2416,2418,2420,2422,2424,2425,2427,2429,2431,2433,2434,2436,2438,2440,2442,
  2444,2445,2447,2449,2451,2453,2455,2456,2458,2460,2462,2464,2466,2468,2469,2471,
  2473,2475,2477,2479,2480,2482,2484,2486,2488,2490,2492,2493,2495,2497,2499,2501,
  2503,2505,2506,2508,2510,2512,2514,2516,2518,2520,2521,2523,2525,2527,2529,2531,
  2533,2535,2536,2538,2540,2542,2544,2546,2548,2550,2552,2553,2555,2557,2559,2561,
  2563,2565,2567,2569,2570,2572,2574,2576,2578,2580,2582,2584,2586,2588,2589,2591,
  2593,2595,2597,2599,2601,2603,2605,2607,2609,2610,2612,2614,2616,2618,2620,2622,
  2624,2626,2628,2630,2632,2634,2635,2637,2639,2641,2643,2645,2647,2649,2651,2653,
  2655,2657,2659,2661,2663,2664,2666,2668,2670,2672,2674,2676,2678,2680,2682,2684,
  2686,2688,2690,2692,2694,2696,2698,2700,2702,2703,2705,2707,2709,2711,2713,2715,
  2717,2719,2721,2723,2725,2727,2729,2731,2733,2735,2737,2739,2741,2743,2745,2747,
  2749,2751,2753,2755,2757,2759,2761,2763,2765,2767,2769,2771,2773,2775,2777,2779,
  2781,2783,2785,2787,2789,2791,2793,2795,2797,2799,2801,2803,2805,2807,2809,2811,
  2813,2815,2817,2819,2821,2823,2825,2827,2829,2831,2833,2835,2837,2839,2841,2843,
  2845,2847,2849,2851,2853,2855,2857,2859,2861,2863,2865,2867,2870,2872,2874,2876,
  2878,2880,2882,2884,2886,2888,2890,2892,2894,2896,2898,2900,2902,2904,2906,2908,
  2911,2913,2915,2917,2919,2921,2923,2925,2927,2929,2931,2933,2935,2937,2939,2942,
  2944,2946,2948,2950,2952,2954,2956,2958,2960,2962,2964,2966,2969,2971,2973,2975,
  2977,2979,2981,2983,2985,2987,2989,2992,2994,2996,2998,3000,3002,3004,3006,3008,
  3011,3013,3015,3017,3019,3021,3023,3025,3027,3030,3032,3034,3036,3038,3040,3042,
  3044,3046,3049,3051,3053,3055,3057,3059,3061,3063,3066,3068,3070,3072,3074,3076,
  3078,3081,3083,3085,3087,3089,3091,3093,3096,3098,3100,3102,3104,3106,3108,3111,
  3113,3115,3117,3119,3121,3124,3126,3128,3130,3132,3134,3137,3139,3141,3143,3145,
  3147,3150,3152,3154,3156,3158,3160,3163,3165,3167,3169,3171,3173,3176,3178,3180,
  3182,3184,3187,3189,3191,3193,3195,3198,3200,3202,3204,3206,3209,3211,3213,3215,
  3217,3220,3222,3224,3226,3228,3231,3233,3235,3237,3239,3242,3244,3246,3248,3250,
  3253,3255,3257,3259,3262,3264,3266,3268,3270,3273,3275,3277,3279,3282,3284,3286,
  3288,3291,3293,3295,3297,3299,3302,3304,3306,3308,3311,3313,3315,3317,3320,3322,
  3324,3326,3329,3331,3333,3335,3338,3340,3342,3344,3347,3349,3351,3354,3356,3358,
  3360,3363,3365,3367,3369,3372,3374,3376,3378,3381,3383,3385,3388,3390,3392,3394,
  3397,3399,3401,3404,3406,3408,3410,3413,3415,3417,3420,3422,3424,3426,3429,3431,
  3433,3436,3438,3440,3443,3445,3447,3449,3452,3454,3456,3459,3461,3463,3466,3468,
  3470,3473,3475,3477,3480,3482,3484,3487,3489,3491,3493,3496,3498,3500,3503,3505,
  3507,3510,3512,3514,3517,3519,3521,3524,3526,3529,3531,3533,3536,3538,3540,3543,
  3545,3547,3550,3552,3554,3557,3559,3561,3564,3566,3568,3571,3573,3576,3578,3580,
  3583,3585,3587,3590,3592,3594,3597,3599,3602,3604,3606,3609,3611,3613,3616,3618,
  3621,3623,3625,3628,3630,3633,3635,3637,3640,3642,3645,3647,3649,3652,3654,3656,
  3659,3661,3664,3666,3668,3671,3673,3676,3678,3681,3683,3685,3688,3690,3693,3695,
  3697,3700,3702,3705,3707,3709,3712,3714,3717,3719,3722,3724,3726,3729,3731,3734,
  3736,3739,3741,3743,3746,3748,3751,3753,3756,3758,3761,3763,3765,3768,3770,3773,
  3775,3778,3780,3783,3785,3788,3790,3792,3795,3797,3800,3802,3805,3807,3810,3812,
  3815,3817,3820,3822,3824,3827,3829,3832,3834,3837,3839,3842,3844,3847,3849,3852,
  3854,3857,3859,3862,3864,3867,3869,3872,3874,3877,3879,3882,3884,3887,3889,3892,
  3894,3897,3899,3902,3904,3907,3909,3912,3914,3917,3919,3922,3924,3927,3929,3932,
  3934,3937,3939,3942,3944,3947,3949,3952,3954,3957,3959,3962,3965,3967,3970,3972,
  3975,3977,3980,3982,3985,3987,3990,3992,3995,3998,4000,4003,4005,4008,4010,4013,
  4015,4018,4020,4023,4026,4028,4031,4033,4036,4038,4041,4043,4046,4049,4051,4054,
  4056,4059,4061,4064,4067,4069,4072,4074,4077,4080,4082,4085,4087,4090,4092,4095
};

