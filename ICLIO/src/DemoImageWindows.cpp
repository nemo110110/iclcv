/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLIO/FileGrabber.h>
#include <ICLCore/Img.h>
#include <ICLIO/File.h>
using namespace icl;
using namespace std;
namespace icl{
namespace{
const int NROWS = 214;
const int NCOLS = 30;
const int NEXTRA = 22;
unsigned char aauc_Data_windows[NROWS][NCOLS] = {
  // {{{ open
  {255,216,255,224,  0, 16, 74, 70, 73, 70,  0,  1,  1,  1,  0, 72,  0, 72,  0,  0,255,219,  0, 67,  0,  6,  4,  5,  6,  5},
  {  4,  6,  6,  5,  6,  7,  7,  6,  8, 10, 16, 10, 10,  9,  9, 10, 20, 14, 15, 12, 16, 23, 20, 24, 24, 23, 20, 22, 22, 26},
  { 29, 37, 31, 26, 27, 35, 28, 22, 22, 32, 44, 32, 35, 38, 39, 41, 42, 41, 25, 31, 45, 48, 45, 40, 48, 37, 40, 41, 40,255},
  {219,  0, 67,  1,  7,  7,  7, 10,  8, 10, 19, 10, 10, 19, 40, 26, 22, 26, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40},
  { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40},
  { 40, 40, 40, 40, 40, 40, 40, 40,255,192,  0, 17,  8,  0,145,  0,214,  3,  1, 34,  0,  2, 17,  1,  3, 17,  1,255,196,  0},
  { 29,  0,  1,  0,  2,  2,  3,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  7,  4,  5,  2,  3,  8,  9,  1,255,196},
  {  0, 68, 16,  0,  1,  3,  3,  2,  3,  5,  6,  2,  6,  8,  5,  5,  0,  0,  0,  1,  2,  3,  4,  0,  5, 17,  6, 33,  7, 18},
  { 49,  8, 19, 65, 81,113, 20, 34, 97,129,145,161, 50,193, 21, 35, 66,114,130,146, 22, 82, 98,115,162,177,178,209, 23, 37},
  { 51, 54,116, 53, 55, 56, 67,225,255,196,  0, 27,  1,  1,  0,  2,  3,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  {  3,  5,  1,  4,  6,  2,  7,255,196,  0, 46, 17,  0,  2,  2,  2,  0,  4,  3,  7,  4,  3,  0,  0,  0,  0,  0,  0,  0,  1},
  {  2,  3,  4, 17,  5, 18, 33, 49, 19, 97,145,  6, 20, 34, 65, 81,113,177,129,193,209,240, 50, 66,161,255,218,  0, 12,  3},
  {  1,  0,  2, 17,  3, 17,  0, 63,  0,245, 45, 41, 74,  1, 74, 82,128, 82,148,160, 20,165, 40,  5, 41, 74,  1, 74, 82,128},
  { 82,148,160, 21,211, 38, 92,120,195, 47,188,134,254, 10, 59,253, 43, 89,171,167, 42,223, 98,125,198,150, 80,242,240,219},
  {100,117,  4,255,  0,249,154,128,193,231,113, 92,238, 41, 74, 82,186,149, 28,154,168,226, 60, 83,221, 31, 36, 99,182, 89},
  {225,240,239,120,173,219, 39,165,216,176, 95,212, 16,154, 25, 73,113,193,230,148,255,  0,189,106,159,214,240, 88, 87,235},
  { 24,127, 30, 99,  6,180, 19, 87,202,222, 62, 21, 19,186,187,146, 71,202,168,165,199,114,155,218,105,126,133,182, 39,  8},
  {162,207,242,223,169,106,219,181,141,146,114,195,105,150, 25,112,244, 75,195,147,239,211,239, 82, 20,144,164,130,146,  8},
  { 59,130, 60,107,204,211, 87,185,173,182,135,213,215,139,109,242, 13,190, 43,190,209, 22, 67,201,104,176,241,247, 83,146},
  {  1, 32,245, 78, 58,237,183,194,172,176,184,212,230,212,110,143,127,154,254,  9,115, 61,154, 74, 14,204,121,118,235,167},
  {252,158,131,165,113,239, 16, 22, 16, 86,158,117, 12,132,231,114, 43,149,116, 71, 34, 41, 74, 80, 10, 82,148,  2,148,165},
  {  0,165, 41, 64, 41, 74, 80, 10, 82,148,  2,148,165,  0,165, 41, 64, 41, 74, 80, 10, 82,148,  4, 27,136,178,185,228, 67},
  {132,147,248, 65,117, 67,215, 97,249,214,170,222,216,  0,109,210,186,111, 82, 61,191, 81, 75,119, 57, 72, 95, 34,125, 19},
  {181,102,176, 57, 25, 42,174, 15,137,223,227, 95, 41, 35,174,174,191,  7, 26, 21,252,245,249, 48,174,110,236,119,168,140},
  {247, 50,163,138,144,221, 93,192, 59,212, 82,106,250,154,175, 93,203,156, 26,244,182,106,166, 47, 57,242,173,207, 10, 45},
  {198,227,171,131,197, 57, 76,100,115,249,225, 74,216,126,127, 74,142, 79, 94, 18,172, 85,161,193,216,130,223,166,230,221},
  { 93, 24, 46, 21, 56, 61, 18, 48,  7,215, 53,117,194,170, 82,185, 73,246, 93,125, 15,124,111, 35,221,240, 38,215,121,116},
  {245,239,255,  0, 54, 99,235,203,163,142,106,172,197,121, 77,251, 54, 18,133, 32,224,130, 54,255,  0, 58,144,216, 53,172},
  {160,132,183,114,104, 63,141,187,196,123,170,249,142,134,171,199, 28, 84,187,139,207,168,146, 86,178,115, 91,184,  8,192},
  {169,165,151,101,114,115,131,211,103,203,103, 54,159, 66,210, 99, 81,219, 93, 78, 75,170, 65,242, 82, 15,229, 93,138,191},
  { 91,128,200,124,171,224, 16,175,246,170,253,145,140, 86, 65,217, 38,189,174, 55,126,181,165,253,253, 66,181,146,137,154},
  {173,134,193, 17,216, 91,135,192,168,242,138,210,194,212,119, 41,250,166,223, 21, 46, 33,166, 20, 86,227,201,  9, 24,228},
  {  9, 61, 73,223,174, 43, 76,241,174,237, 24,207,180, 93,110, 19, 58,165,190, 88,200,245, 30,241,255,  0, 49, 83,225,229},
  { 95,149,122, 82,151, 69,215,160,174,114,156,252,139, 65, 10, 74,210, 20,147,144,124,105, 95,141, 32, 54,210, 80, 58, 36},
  { 98,149,209, 27,  7, 42, 82,148,  2,148,165,  0,165, 41, 64, 41, 74, 80, 10, 82,148,  2,148,165,  0,172, 43,212,207, 96},
  {181, 74,147,226,218,  9, 79,175, 65,247,172,218,143,235,120,242,229,218, 16,196, 38, 84,233, 91,169,231,  9, 35, 60,163},
  { 39,196,143, 28, 84, 57, 18,148, 42,147,130,219,215, 66,124,104, 70,118,198, 51,122, 91,234, 64, 45,173,248,158,181,184},
  {144,123,182, 64,174, 48,172,179, 82,129,222,188,204,116,252,  7, 58,191, 32, 62,245,152, 45,145, 18,115, 34,124,133, 18},
  {115,239, 58, 16, 62,192, 87, 41, 95,  4,202,185,237,173, 29,  6, 79, 18,199, 83,223, 54,254,196, 50,236,239, 95, 58,141},
  { 77, 87,218,173, 85, 88,237,114,128, 74, 36,239,184,218, 64, 61,124,185,179, 90, 59,222,128,144, 27, 82,226,149, 57,182},
  {192,128,149, 31,159,225, 63,106,197,188, 15, 38,158,169,111,236, 88,225,113,220, 71,168,201,235,238, 84,243, 74,156, 80},
  {109,  3, 42, 81,192, 30,100,237, 87,101,217, 41,211,220, 61,143,  9,  7, 11, 83,105,107,111, 18,  6,231,249,142,106,186},
  {211,186,118, 90,181,164,  6, 38, 50,160,211,107, 46,172,145,224,157,192, 57,243, 35,239, 83, 78, 40, 75,230,155, 18,  2},
  { 78, 67, 41,247,135, 93,250,159,203,233, 91, 56,144,240,177,167, 55,221,190, 95,228,211,246,171, 50, 51,240,233,131,218},
  {214,253,122, 47,220,137,219,219,233, 82, 72,136,194, 69,105,173,205,244,173,252,116,224, 10,175,190, 71, 15, 55,212,202},
  {104, 87, 55, 14, 19, 70,198,  5,112,124,212,  8,199,100, 97, 75,117, 45, 52,183, 20,125,212,130, 79,202,165, 58,  6, 17},
  { 98,213, 12, 56, 61,247, 50,251,158,170,223,253,133, 67,110, 13,153, 74,143, 13, 35, 38, 75,169,108,143,236,245, 87,216},
  { 26,181,108,237,  4,165, 74,  3,  9, 30,232,174,147,131, 85,165, 43, 31,216,155, 29,116,108,217, 82,148,171,194,113, 74},
  { 82,128, 82,148,160, 20,165, 40,  5, 41, 74,  1, 74, 82,128, 82,148,160, 21,  2,213, 26,162, 98, 47, 47,219, 32,150,208},
  {211,120, 74,220,198, 84, 73,  0,145,232, 51,138,157, 72,117, 44, 48,227,171, 32, 33,180,149, 18, 78,  6,  0,205, 83, 48},
  { 86,169,215, 23,228,175, 60,206,184, 87,191,196,214,222, 37,106, 91,148,151, 99, 87, 38,110, 58,140,126,100,178,218, 86},
  {180, 23, 93, 82,150,172,117, 81,172, 91,131,155, 26,207,108,119, 81,  7,198,180,215,  7, 58,253,106,120,173,200,134, 79},
  { 72,209,220,156, 30,247,144,173, 76, 93, 99,117,176,184, 12, 89,  5,198, 70,197,135,143, 50, 15,203,195,229, 89, 23, 71},
  { 48,149,121,250, 84, 30,248,254,202,223,195,198,172,169,170, 51, 92,178, 91, 35,171,123,218, 61,  9,161,175,240,245,141},
  {189, 87,  6, 98, 46, 51,204, 57,221, 58,  8,200, 42,192, 56,  7,196,110, 13, 67,181,213,146,226,197,233,233,210, 19,207},
  { 29,213, 18,151, 17,184,  3, 61, 15,151,207,111,141,108,244,  3, 46,105,238, 26,165,194, 74, 29,120,119,167,127,219, 94},
  {255,  0, 80,156,125,171, 50,199,170, 95, 45,119, 23, 68,123, 75, 71,110,110,138,  3,227,231, 92, 63, 25,202,196,175, 33},
  {209, 54,210, 93,159,125,124,139,186,177,103,117,124,200,136,219,219,198, 43,114,210,112, 42, 74,237,146,203, 60,247,176},
  {157, 17,156, 57,247, 82,121, 51,242,232,126,149,140,173, 46,250,119,106,106,138,113,144,121, 82,175,242,197, 82,188,  9},
  { 93,241, 81, 56,201,121, 51, 78,120,211,139,234,107, 64,192,172, 73, 46, 37,  3, 43, 80, 72,243, 38,183, 39, 79,172, 28},
  { 63,112,116,124, 18, 18,138,252, 69,166,217, 29, 92,235,105, 82, 22, 55,230,121, 69,120,250,237, 80,217,141, 94, 55, 92},
  {155, 99, 31,215,175,161,234, 56,147,179,162, 52,250,105,131, 54,253,237, 92,164,199,142,210,130, 85,141,138,212,113,254},
  { 64,253,106,208,132,142,238, 50,  7,137,220,252,235, 85, 97,101,153,214,214,100,132, 41,  8, 94,121, 81,140, 12,  2, 71},
  {211,106,222, 87, 87,135, 84,106,166, 42, 15,107,191,169,233, 67,147,225,250, 10, 82,149,178,100, 82,148,160, 20,165, 40},
  {  5, 41, 74,  1, 74, 82,128, 82,148,160, 20,165, 40,  8,230,191,155,236,122,106, 66, 82,112,185, 24,100,122, 30,191,108},
  {212, 27, 79, 71,217, 27, 98,182,252, 78,149,223, 92, 96, 65, 73,247, 91, 73,117, 99,204,157,135,216, 31,173,113,177, 51},
  {202,144, 72,232, 42,202,165,201, 74,243, 52, 45,124,214,253,140,233,170,229, 64, 79,149, 70,238, 46,117,173,220,247, 63},
  { 21, 70,110, 46,117,248, 84,148,196,142,198, 71,174,238,245, 27,212, 77, 17, 85,118,189,195,128,222,115, 33,228,160,159},
  { 32, 78,231,233, 91,203,195,191,143,124,254,117,155,193,251,113,184,235, 23, 37, 41, 57,110, 35, 94,233,254,218,142,  7},
  {219,154,172,185,252, 26,165,103,209, 18, 81, 29,178,204,214,110,136,214,184, 80, 90,  1, 41, 35,159,148, 12, 96,116,  3},
  {232, 13,104, 96, 35,165,100,234,185, 34, 93,249,192,131,148, 55,132, 39, 31, 13,191, 42,253,130,142,149,241, 14, 51,147},
  {226, 91, 57,249,254, 58, 29,134, 60, 60, 58,146, 54,177, 19,210,182,140, 13,171,  6, 50,113, 91, 22,134,  5,115, 56, 81},
  {231,177,204,197,172,252,124,242,162,180,215, 87, 75,112,157, 41,252,106, 28,137,245, 59, 10,218, 74, 86,248,173, 83,169},
  { 18, 46,176,152, 59,132,146,242,135,238,244,251,159,181, 79, 10,189,247,136,215, 66,237,180, 97, 62, 72, 57, 50, 85, 96},
  {105, 77, 52,203, 60,199,145,150,194, 64,  7,106,220,214, 29,173, 28,177,202,143,237, 28,214,101,125,150, 49, 81, 73, 34},
  {133,189,189,138, 82,149,147,  2,149,  0,227,202, 11,188, 42,189, 52,151, 92,107,189, 84,102,202,219, 87, 42,146, 21, 33},
  {160,112,125, 13, 66, 33,234, 75,147,124, 35,123, 74,123, 66,142,169,102,111,244, 97, 11,230,247,250,224, 61,158,191,244},
  { 61,252,252, 50,104, 11,218,149, 64,240,214,127,176,104,238, 21, 70,144,217,150,183,174, 83,217, 18, 29,113,124,232,229},
  { 91,251,236,112, 73,  3, 24, 57,248, 84,194,195,172,181,109,237,152,151,235,125,158,222,254,153,149, 51,217,209, 29, 10},
  { 95,182,134,123,194,223,126, 78,121, 49,145,204, 83,140,242,248,237, 64, 89,212,170,123,254, 38,106, 22,161,106,155,212},
  {139, 93,181, 54, 27, 28,199,160,164,  7, 23,223,203,116, 56, 16,216, 78,248, 72,202,144,  9, 57,234,112,  6, 43,100,230},
  {181,213, 26,122,227, 14, 62,174,182,219, 30, 23, 40,143,189, 15,244, 90,156,230, 75,205, 55,222, 22, 20, 23,158, 98, 71},
  { 69, 12,110, 14,212,  5,159, 74,169,184,125,196,123,150,161,149, 17,217, 82,244,188,136, 47,176,183,100, 49,  6, 66,211},
  { 42,  1, 72,206, 28, 66,206, 87,228,112, 19,131,191, 74,233,103,136,218,149, 26,118,223,172,166,219,109, 72,210, 51, 36},
  { 54,142,229, 11, 89,150,211, 14, 56, 27,109,210,163,238, 29,212,146, 82,  7, 67,215,198,128,183,137,  0,128, 72,201,232},
  { 43,246,168,107, 77,191, 82, 77,214, 58,254,244,180,216,157,186, 91,213,220, 70,144,224,120,169,129,220, 36,128,222, 14},
  {195,149,106, 39,199,152,249, 84,139,135,111,235,203,134,144,209, 83, 29,153,110,122, 51,172, 48,236,149,251,202,117,198},
  { 74, 16,114,225, 94, 73, 89, 28,219,167,198,128,181,233, 74, 80, 20,189,226,228, 46, 26,178,224,227, 77,189, 37,104,120},
  {182,150,216,108,172,128,159,116,103,192,116,206,248,173,228, 33,125, 83, 33, 44,219,163,197,  7,108,201,127, 42, 31,194},
  {128, 71,222,172, 55,161, 52,235,156,202, 24,243,  9,219, 39,204,215, 36, 68, 97, 29, 27,  7,215,122,219,150, 86,210, 74},
  { 61,136, 35, 68, 83,219, 43,199,109, 55,199, 71,189, 46,214, 14,122,  6, 28, 63,126,127,202,180,183, 27,  5,241,  8, 42},
  {238, 99, 73,  3,124,176,231, 42,190, 73, 86,223,122,184,131,104, 29, 16,145,242,161,105,181,117, 66, 79,168,172, 71, 46},
  {200,253, 61, 15, 78,152, 62,232,242,205,248,184,202,150,135,155,113,181,143,196,133,140, 40,124,143,165, 89,124, 25,133},
  {250, 55, 73,201,186,184,159,214, 72, 82,157, 25,242, 30,234,126,167, 38,172, 43,254,151,181,223, 99, 22,103, 71,  7,250},
  {171, 27, 41, 62,135,194,181,119, 91, 66,237,154, 83,216,109,172,158,233,188, 32, 37, 27,144,128, 63,223, 21,142, 39,196},
  {155,194,146,130,248,191,191,185, 38, 53, 41, 88,147,236, 65,154, 37,233, 11,112,239,204,172,214,246, 18, 54, 21,168,130},
  {209, 24,205,111,226, 39, 97, 95, 22,226,118,234, 58, 71, 80,204,248,233,216, 86,112,217, 53,143, 29, 53,222,225,229, 69},
  { 71,135, 31, 14,174,102,106,205,237,232,195,125, 89, 81,172,107, 26, 11,215,  9,178, 55, 32, 16,202, 62, 93,126,244,150},
  {240,105,165,184, 79, 64, 79,173,118, 91,222,253, 15, 99,110, 83,177,222,127,144,  7, 92,109,188,115, 18,165,124, 72, 29},
  { 77, 91,251, 29,142,239,204,158, 76,191,213,126, 72,115,101,203, 87, 47,212,155, 50,142,237,164, 39,200, 98,185, 86,167},
  { 76, 95,163,106, 59,103,183, 66,110, 67,108,243,114, 97,246,202, 21,156,  3,208,250,227,229, 91,106,250,105, 80, 41, 74},
  { 80, 17,222, 33, 88,206,163,210, 83, 45, 98, 83,113, 59,213,178,190,249,193,148,167,145,212, 47,125,199, 94, 92,124,234},
  { 60,222,131,183,187,197, 85,106,230, 46, 13,175, 49, 59,179,  9, 39, 35,191,  0,160, 61,177,235,200, 74,122,103,227, 82},
  { 46, 33, 89,  6,163,208,247,203, 70, 50,169,113, 28,109, 31,191,140,167,238,  5,120,115,179,254,161,123, 79,113,134,194},
  {185, 47, 44, 51, 33,243,  9,224, 85,183,235, 50,145,159, 69, 16,104, 15, 92,216,248,104,253,186,221,163, 35, 46,226,211},
  {134,193, 54, 84,167,  8,104,142,248, 60, 93, 60,163,125,177,222,124,241, 93, 22,173, 27,122,177,136,214, 40,250,174, 44},
  { 93, 53, 26,103,181,161, 13,160,183, 48,163,188,231, 17,202,194,241,201,147,130,113,146, 54,198, 13, 90, 85,243,167,141},
  { 26,133,219,255,  0, 20,117, 28,228, 62,178,209,148,166, 91,194,182,228, 71,184,159,176,160, 61,168,207, 14,155,147,163},
  { 53, 69,138,100,240,182,239,115,223,158,219,236,163,118, 74,214,149,160,238,119, 41, 82, 65,248,226,186,  6,139,212, 23},
  { 43,140, 59,142,171,212, 81, 67,182,184,175, 51,  5,118,248,197,174, 71, 92, 71, 33,146,178,181, 40,115,128, 54, 78, 48},
  { 50,122,214,179,178,214,160, 55,238, 17,192,109,213,243,191,110,117,112,151,147,147,129,239, 39,252, 42, 77,112,237, 83},
  {168, 13,143,132,115, 89,105,194,220,139,147,173,196, 65,  7,  7,  4,243, 43, 31,194,147,245,160, 59, 99,104, 73,183, 45},
  { 65,105,184,106, 75,166,158,125,187, 66,148,233,145,  6, 16,106, 76,188,160,163, 15, 44,168,242,167, 10, 57,  3,169,  2},
  {184, 91,248,119,116,122,205, 11, 74,200,212,112,101,105, 24, 50, 27,121, 45,182,193,246,183, 91, 66,249,219,105,106,230},
  {229,  0, 41, 35,222,  3, 39,151, 24, 21, 72,240, 87, 70, 93, 47,124, 18,215,114,237,172,185, 34,227,113, 40,133, 21, 28},
  {248,230,  8, 41, 82,176, 73,248,143,165, 79,187, 44,104, 45, 87,163,239,119,231,245, 77,185,232,109, 72,142,218, 26, 46},
  { 58,149,243, 40, 40,147,208,159, 58,  2,229,179,105, 99,  2,118,171,117,217, 41, 91, 87,183,195,161, 41, 78, 11, 67,185},
  { 75,100, 29,247, 62,238,107,143, 14,108,119,141, 55,167,163,217,174,210,224,203,141,  1,164, 70,134,228,118,148,133,169},
  {164, 12,  2,230, 73, 28,216,229,233,182,198,165, 84,160, 20,165, 40,  5,117,188,251, 44, 99,190,117,182,243,211,157, 64},
  {103,235, 92,148,226, 18,112,165,164, 31, 34,107,203,189,183, 92, 90, 27,210,124,138, 82,114,100,116, 56,240, 69,  1,234},
  { 38,220, 67,168, 11,109,105, 90, 79, 66,147,145, 92,170,166,236,176,165, 47,130, 86, 53, 45, 69, 74, 46, 73,220,156,255},
  {  0,247,174,173,154,  1, 74, 82,128,143,220,116,220,119,212, 92,143,250,165,158,188,187,103,242,173, 74,237,115,226,146},
  { 18, 91,115,203,157, 37, 63,122,155, 82,171, 50,248, 54, 22, 99,221,213,166,253, 63,  4,240,201,178, 29, 19, 33, 73, 23},
  { 20,228,  6, 99,143, 85,147, 95,170,141, 61,253,157,144,211, 73,242,105,  4,147,245,169,145,105,179,213,  9, 62,162,129},
  {180, 36,228, 33, 35,208, 86,189,126,207, 96,195,167, 46,215,155,103,167,151, 99, 34,145,108, 73, 42, 10, 90, 28,125,127},
  {214,119,167,251, 84,130,  4, 62,224, 18,231, 41, 36, 96,  0, 58, 86,109, 42,214,156,122,168,143, 45, 81, 73,121, 16, 74},
  {114,155,220,158,192,  0, 13,128, 30,148,165, 42, 99,200,165, 41, 64, 43,231,143, 26,108,235,210, 60, 96,190, 51, 29, 61},
  {218, 19, 51,219, 35,129,208, 37,103,188, 72, 31,  0, 78, 62, 85,244, 58,188,145,219, 98,193,220, 95, 52,253,253,164,251},
  {178, 89, 92, 71, 72, 31,180,131,204,156,252, 72, 81,254, 90,  3,208, 83,245,147, 77,240,133,221, 92,218,210, 65,181,123},
  { 98, 15,129, 89,111, 32,127, 49,  2,188, 53,165, 52, 99,250,139, 68,235, 13, 65,149,169, 86,118,218,117, 39, 57,231, 42},
  { 95,191,159,225,201,169,243,250,235,155,178,164,123, 23,123,153, 98,230, 96, 17,227,221,100,189,159,184, 31, 49, 87, 31},
  {102, 61, 34,203,156, 12,148,212,196, 96, 95,214,249,112,227,126,236,142,233, 63,233, 36,122,208, 21,255,  0, 98,155,249},
  { 98,255,  0,125,176, 56,177,201, 41,148,202,109, 63,218, 65,194,190,202, 31, 74,253,237,177,126,239,239,186,126,194,210},
  {253,200,172,174, 83,160,120,169,100, 37, 57,244,  9, 63,205, 85, 95,  9, 46, 79,104,126, 52, 90, 76,191,213,152,211,213},
  {  6, 80, 59,224, 40,150,151,244,206,126, 85,188,215, 78, 43,136,221,164, 93,134,202,139,145,223,186, 55,  1, 36,120, 52},
  {218,130, 20,161,240,194, 84,170,  2,246, 48,166,104,110,201,192, 91,228, 57, 14,226,220, 20,201, 47, 52,162,149, 37,110},
  {184, 22,119,244, 86, 62, 85, 26,236,133,171, 47,250,138,253,168,154,190,221,230,220, 27,102, 51, 74,109, 50, 29, 43,  9},
  { 37,103, 36,102,173,110,208,200, 75, 92, 19,212,141,182,144,148, 38, 50, 82,144, 14, 48,  2,211,181, 81,221,136,191,238},
  { 77, 79,255,  0,136,215,250,205,  1,162,213, 28, 74,191,216,248,253, 45, 18,245,  5,201,187, 12, 59,216,239, 99,165,226},
  { 81,220, 37,193,204,158, 92,244,198, 70, 43,150,162,145,197,206, 41,137, 58,146,209, 26,232,197,132, 21, 24,172, 70,127},
  {185, 79,118,  9,252, 35, 32,172,249,157,243,244,168, 79, 20, 34,137,252,119,190, 68, 87,225,145,121, 45, 31, 69, 56,  7},
  {231, 95, 65, 96,197,102, 12, 40,241, 34,182, 27,142,195,105,105,180, 14,137, 74, 70,  0,250, 10,  3,197,252,  2,227, 22},
  {161,179,107, 43,125,135, 80,207,145, 58,209, 49,225, 20,166, 90,138,151, 29,100,225, 42, 74,142,253,112,  8, 63,231, 87},
  {167,105, 94, 38,200,208, 26, 93,136,182,103,  2, 47,151, 62,100,178,225,  0,247,  8, 78, 57,151,131,227,190,  7,199,210},
  {188,185,174, 91, 76, 94,208,151, 20,199,  1,180,162,254, 10, 66,118,193,239,129,169, 71,108, 73,142,200,226,211,108, 57},
  {158,238, 53,189,148, 32,120,110, 86,162,126,170,251, 80, 17,205, 17,160,120,129,196,227, 42,239,110,125,231, 80,219,135},
  {154,100,217, 74, 64, 91,157,112,147,185, 36,109,211, 97,181, 96,241, 86,225,172, 35, 55,110,210,154,237, 46, 25,150,114},
  {181, 50,235,171,231, 90,155,112, 39,  3,156,126, 36,142, 93,143,199, 30, 21,237,110,  7,219,217,182,112,143, 73,179, 28},
  {  0,151, 45,204,200, 87,197, 78, 36, 45, 95,117, 26,163,123,111,176,208,119, 74, 72,  8, 29,242,146,251,101,126, 37, 35},
  {144,129,245, 39,235, 64, 88,157,155,238, 81,108,221,158,173,215, 43,139,161,152,113, 68,183,157,112,254,202, 82,243,132},
  {154,161,181,103, 22,181,223, 20,181, 80,179,232,245, 76,133, 13,229,148, 70,133,  8,242, 56,164,255,  0, 89,197,141,250},
  {110,119,  0,125,235,127, 50,115,208,251, 22,219,155, 97, 92,162, 92,245, 48,225,  7,  4,167,218, 92, 81, 31,224, 31, 44},
  {214,199,177, 13,186, 59,147,181, 93,197, 73,204,150, 91,142,194, 15,146, 86, 92, 42,250,148, 39,233, 64, 64,245, 21,139},
  {139,220, 50,142,213,238,124,251,155, 17,210,176, 11,205,206,246,132, 36,158,129, 99, 36, 96,159, 49,138,184, 52,143, 19},
  {238, 60, 80,225, 30,164,130,196,149,192,214, 86,216,133,254,120,167,144,188, 19,239,  5,163, 29, 51,140, 17,230, 71,157},
  { 93,218,242,220,205,219, 68,223,160, 74,  0,179, 34, 11,205,171, 62, 25, 65,193,249, 29,254, 85,226, 94,203,115,220,131},
  {198,123, 58, 27, 81,  8,148,135,163,184,145,209, 73, 40, 39,  7,230,144,126, 84,  4,235,178,223, 18,175,151, 14, 35,155},
  { 54,162,188, 76,156,196,248,203, 12,  9, 46,149,242,186,129,207,182,122,101, 33, 95, 65, 86,143,106,237, 99, 59, 74,232},
  { 72, 45, 89,230,189, 14,225, 62, 88, 66, 93,101, 92,171,  8, 64, 42, 86,254,188,163,231, 94, 98,212,145,220,225,127, 28},
  { 94, 44, 37, 72,110,215,114, 76,134, 65, 29, 89, 36, 40, 15,228, 56,171, 47,180,172,223,233,199, 23,244,150,154,128,190},
  {245,130,219, 40, 71, 41,219,154, 66,193, 42,207,238,132,125, 40,  9, 15,100, 93,127,119,191, 94,175,182,109, 67,116,147},
  { 61,222,225, 18,163,153, 14, 21,148,132,171,149, 96, 19,251,200,172,254,215,122,246,231,167, 17, 97,180,216, 46, 82, 96},
  { 75,120,174, 75,235,142,190, 82, 80, 61,212,130, 70,253,121,143,202,170,141,  6,131,195, 78,211, 13,219,  9, 41,140,220},
  {245, 64,202,207, 86,157,217,  7,232,164, 26,203,226, 54,120,155,218,109, 22,166,207,123, 13, 18,155,128, 48,115,134,154},
  {221,223,191,120,126,116,  6,203, 95,113, 87, 85,233,205, 49,167,180,133,178,227, 43,244,219,176,218,147,113,156,181,115},
  { 60, 92,119,222, 75, 73, 81,221, 56,  4,100,245,240,219,122,236, 71,  5,248,197, 34,  0,185,171, 81,114,201, 82, 59,209},
  { 25, 87, 87,131,217,234,  6,195,148, 31,226,172, 62,213,218, 26,239,108,215, 46,106,104,145,159,118,211, 45,182,207,126},
  {210, 73, 17,220, 66, 66,121, 84, 71,225,232,  8, 39,242,173,102,151,237, 41,174, 44,209,217,141, 48, 91,238,204, 54,  2},
  {115, 41,162,151,  8,  3, 31,137,  4,111,241, 32,208, 18,142, 18,234, 93,119,170,231,255,  0, 67,181, 82,175, 43,129,222},
  {156, 92,  3, 42,231,142,226, 51,238, 56,177,142,100, 31,137,200, 32, 17,229, 94,183,142,215,115, 29,166,185,214,231, 34},
  { 66,121,214,114,165, 96, 99, 39,227, 84,111, 10,123, 68,218,117,141,230, 61,154,241,  1, 86,155,140,131,200,194,131,157},
  {227, 78, 47,250,185,192, 41, 39,195, 59,124,106,246,160, 20,165, 40,  5, 84,189,168,236, 31,167, 56, 67,116,117,180,243},
  { 63,110, 82,102,167,  3, 39,149, 39, 11,255,  0,  9, 39,229, 86,213, 98, 93,224, 51,117,180,205,183,203, 72, 84,121,108},
  {173,135,  1, 25,202, 84,146,147,246, 52,  7,204,  6,123,231,139,113,155, 82,212, 20,191,117,188,237,204,118,206, 60,250},
  { 87,210,253, 17,102, 78,158,210, 22,107, 66,  6, 61,142, 35,108,159,222,  9, 25,251,230,169, 61, 59,217,126,209,103,191},
  {219,238, 78,106,  9,114,145, 18, 66, 31,238, 23, 25, 32, 57,202,174, 96,146,115,211, 97, 94,135,160, 60, 19,218,114,196},
  {173, 61,198, 27,147,140,165, 77,181, 60, 34,115, 74, 27,110,173,149,131,251,201, 85, 72, 59, 31, 88,213,118,226,100,171},
  {179,193, 75, 69,178, 50,156, 42, 59,254,177,195,202, 50,124,241,206,126, 85,232, 78, 50,240,118,223,196,233,150,217, 82},
  {110, 79, 91,164, 66, 66,218,230,105,160,190,241, 42, 32,224,228,142,132, 28,122,154,206,224,223, 11, 96,240,198,223,113},
  { 98, 28,231,103,189, 57,196, 45,199,156,108, 32,225, 32,132,167,  0,157,134, 84,126,116,  7, 14,209, 95,251, 49,169,255},
  {  0,184, 79,134,127,109, 53, 70,118, 34,255,  0,185, 53, 63,254, 35, 95,235, 53,233,173,127,166, 90,214, 58, 66,229, 97},
  {126, 66,227, 55, 53,176,130,242, 19,204, 81,130, 14,113,227,210,161,124, 27,224,236, 78, 24,220,110, 82,226, 93,223,158},
  {102,180,150,138, 93,100, 35,147,148,231, 59, 19,158,180,  7,148,117,238,253,162,110, 61, 15,252,253, 61,127,189, 21,244},
  { 10,168,171,231,103,123,125,219, 95, 72,213, 11,191,202,109,215,167,  9,166, 56,142,146,144,121,130,185,115,156,227,106},
  {189,104, 15,159,188, 65,255,  0,228, 69,211,167,254,188, 58,255,  0,122, 42,197,237,169,167, 93,103, 83, 89,117, 19,104},
  { 81,141, 38, 55,177,184,175,  0,226, 20,165, 15,170, 85,254, 26,178,111,221,157,237,247,125,125, 39, 84, 46,255,  0, 45},
  {167,158,156, 38,150,  4,116,148,131,204, 21,203,156,244,218,173,173, 99,166, 45, 90,194,193, 34,207,125,140, 36, 67,120},
  {122, 41, 10, 29, 20,147,224, 71,157,  1, 79,118,121,226,214,154,119,135, 86,235, 69,242,239, 18,219,114,181,181,236,234},
  { 68,183, 67, 97,198,211,248, 84,146,118, 59, 96, 99,174,213, 77,118,164,226, 21,191, 91,106, 56, 49,108, 46,123, 77,174},
  {214,149, 35,218,210, 61,199, 92, 86, 10,185, 79,144,192, 31, 31, 74,176,101,246, 76,136,185,220,209, 53,107,237, 67, 39},
  { 61,219,144,  2,214,  7,151, 48,112, 15,159, 45, 76, 47,189,157, 52,244,237, 31,106,176, 91,167,200,128,152,111, 45,247},
  {101,119, 73,113,201, 43, 80,  0,149,244,232,  6,216,233, 64, 68,244,126,149,127, 88,118, 63,106,219,  9,  5,115, 82,167},
  {228,199, 72,234,165,162, 66,213,129,241, 35, 35,231, 85,191,101,254, 33, 64,208,154,178,225, 11, 80,189,236,150,219,146},
  { 18,133,186,176,112,203,168, 39,148,171,200,123,202,  4,248,109, 94,186,225,150,143,107, 65,232,200, 90,122, 60,181,204},
  {110, 50,156, 80,121,104,  8, 42,231, 90,151,208, 19,211,155, 21, 94,241, 51,179,198,156,214, 55, 39,174,118,233, 46, 88},
  {238, 47,168,173,213, 50,208,113,167, 20,122,168,183,145,191,161, 25,160, 57,113,171,140, 90, 98,213,160,238,113,172,183},
  {152, 87, 27,172,246, 23, 25,134,162, 58, 29, 40,231, 78, 10,212, 70, 64,  0, 18, 70,122,156, 85, 49,216,243, 73,201,185},
  {107,199,181, 19,141,173, 54,251, 91, 43, 66, 92,198,203,121, 99,148, 39,228,146,162,126, 94,117, 55,178,118, 79,183, 49},
  { 49, 46, 94,181, 60,137,177,193,207,115, 30, 32, 96,168,124, 84, 86,175,176,175, 66,233,157, 63,107,211, 22,104,246,171},
  { 28, 54,162, 66,100, 97, 45,160,117, 62, 36,159, 18,124, 73,160, 60,177,219, 83, 77,251, 62,160,178,234, 54, 27, 33, 18},
  {216, 49, 31, 32,109,206,131,148,147,241, 41, 81, 31,194, 42, 51,217,114,223, 39, 83,241,134, 29,198,122,214,248,181, 68},
  {239, 57,148,122,114,160, 52,216,249,  2, 62,149,234,254, 43,232, 40,124, 69,210,166,205, 54, 66,226, 16,242, 30,110, 66},
  { 16, 22,164, 20,245,216,249,130, 69,105, 56, 51,194, 56, 60, 48, 85,213,200,183, 23,174, 15, 79, 13,165, 75,117,160,142},
  { 64,142,109,134,  9,234, 85,191,160,160, 60,251,219, 10,204,229,163,137,112, 47,177, 66,155,246,248,200, 95,120,145,140},
  { 58,209,229,206,124,241,201,244,172,238,199,118, 87,111, 26,246,247,169,230,133, 58,168,141, 20,135, 85,226,243,196,228},
  {250,224, 43,249,170,254,227, 39, 11,160,241, 58,217,110,141, 46,115,176, 29,132,242,156,109,230,219, 11, 36, 41, 56, 82},
  {112, 72,234, 66, 79,202,178,184, 67,195,152, 92, 53,211,210, 45,112,101,185, 53, 82, 36, 25, 14, 62,226,  2, 20,125,208},
  {  2,112, 60,  6, 62,230,128,133,107, 30,208,186,115, 79,106,183,244,244,251, 77,197, 74, 98, 79,179,202,117,196,164, 33},
  {  9,200,202,192,201, 42, 24,220,121,214,230,237, 98,224,254,164,183,170,225, 41,157, 46,182, 22,158, 99, 37,151, 27,101},
  { 91,248,146,146, 14,125,119,172,206, 40,240,115, 76,241, 13,193, 46,224,219,144,174,161, 60,162,108, 92,  5,168, 14,129},
  { 96,236,172,124,119,248,213, 68,174,201, 41,239,178,157,102, 67, 89,232,109,158,247,166,123,223,191,218,128,161,167,219},
  { 96,171,138,191,163,244, 35,239, 74,133,250, 69, 13,219,221, 25, 43, 87,188, 48, 65,192,206, 15,143,144,205,125, 31,170},
  {183,133,188, 18,211, 60, 63,148, 39,199, 14,220,110,193, 37, 41,151, 43, 31,171,207, 94, 68,141,147,235,185,248,213,165},
  { 64, 41, 74, 80, 10, 82,148,  2,148,165,  0,165, 41, 64, 41, 74, 80, 10, 82,148,  2,148,165,  0,165, 41, 64, 41, 74, 80}
};
// }}}
unsigned char auc_ExtraData_windows[NEXTRA] = {
  // {{{ open
 10, 82,148,  2,148,165,  0,165, 41, 64, 41, 74, 80, 10, 82,148,  2,148,165,  1,255,217
};
// }}}

}//end namespace
ImgBase* createImage_windows(){
  // {{{ open
  static ImgBase *image = 0;
  if(image) return image->deepCopy();
  File f("./.tmp_image_buffer.jpg",File::writeBinary);
  const int DIM = NROWS*NCOLS+NEXTRA;
  char *buf= new char[DIM];
  int j=0;
  for(int i=0;i<NROWS;++i){
     for(int k=0;k<NCOLS;k++,j++){
        buf[j] = aauc_Data_windows[i][k];
     }
  }
  for(int i=0;i<NEXTRA;i++,j++){
     buf[j] = auc_ExtraData_windows[i];
  }
  f.write(buf,DIM);
  f.close();
  delete [] buf;
  image = FileGrabber("./.tmp_image_buffer.jpg",false,true).grab()->deepCopy();
  f.erase();
  return image->deepCopy();
}
// }}}

} // end namespace icl



