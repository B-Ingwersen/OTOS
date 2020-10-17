#include "CStandardLibrary/math.h"

#define EULERS_NUMBER 2.7182818284590452353602874713526624977572470936999595749669679
#define CONSTANT_PI 3.1415926535897932384626433832795028841971693993751058209749445
#define CONSTANT_2PI 6.283185307179586476925286766559005768394338798750211641949889
#define NATURAL_LOG_2 0.69314718055994530941723212145817656807550013436025525412067998
#define NATURAL_LOG_10 2.3025850929940456840179914546843642076011014886287729760333285
#define LOG_2_E 1.4426950408889634073599246810018921374266459541529859341354492
#define LOG_10_2 0.30102999566398119521373889472449302676818988146210854131042735
#define SQUARE_ROOT_E 1.6487212707001281468486507878141635716537761007101480115750791
#define SQUARE_ROOT_2 1.4142135623730950488016887242096980785696718753769480731766796
#define DOUBLE_ERROR 0.0000000000000001
#define NAN 0x7FFFFFFFFFFFFFFF
#define INFINITY 0x7FFFFF0000000000
#define SIN_PI_OVER_8 0.38268343236508977172845998403039886676134456248562704143380049
#define SIN_3PI_OVER_8 0.92387953251128675612818318939678828682241662586364248611509802
#define SIN_PI_OVER_16 0.19509032201612826784828486847702224092769161775195480775450208
#define SIN_3PI_OVER_16 0.55557023301960222474283081394853287437493719075480404592415378
#define SIN_5PI_OVER_16 0.83146961230254523707878837761790575673856081198724996344612457
#define SIN_7PI_OVER_16 0.98078528040323044912618223613423903697393373089333609500291634

const double exp_refTablep[] = {
    2.7182818284590452353602874713526624977572470936999595749669679,
    7.3890560989306502272304274605750078131803155705518473240871285,
    54.598150033144239078110261202860878402790737038614068725826588,
    2980.9579870417282747435920994528886737559679391328357022089624,
    8886110.5205078726367630237407814503508027198218566388397839918,
    78962960182680.695160978022635108224219956195115352330655079996,
    6235149080811616882909238708.9284697448313918462357999143885933,
    38877084059945950922226736883574780727281750630829988857.729675,
    1.5114276650041035425200896657072865075062408982871207163163516e+111,
    2.2844135865397566403787515171224034228968101878058981082429136e+222,
};

const double log_refTable[] = {
    1.6487212707001281468486507878141635716537761007101480115750791,
    1.2840254166877414840734205680624364583362808652814630892175078,
    1.1331484530668263168290072278117938725655031317451816259128204,
    1.0644944589178594295633905946428896731007254436493533015193071,
    1.0317434074991026709387478152815071441944983266418160960083487,
    1.0157477085866857474585350720823517489067162884821700349446288,
};

const double eToThe16ths_referenceTable[] = {
    0.47236655274101470713804655094326791297020357913647668239565781 ,
    0.50283157797094095968863661143762517988284294280109943919615221 ,
    0.5352614285189902419566225080220464054335441250360218912489004 ,
    0.5697828247309230097666296898291228158846384743279959772910084 ,
    0.60653065971263342360379953499118045344191813548718695568289198 ,
    0.64564852642789203734835568006103194636093977727957575451738942 ,
    0.68728927879097219854520233914651359043465202377252106918265674 ,
    0.73161562894664179115955942049140282528128115321984719284439311 ,
    0.77880078307140486824517026697832064729677229042614147424131735 ,
    0.82902911818040034301464550934308186242538840928345113275699072 ,
    0.88249690258459540286489214322905073622200482499065074177030925 ,
    0.93941306281347578611971082462230508452468089054944182200949237 ,
    1.0 ,
    1.0644944589178594295633905946428896731007254436493533015193071 ,
    1.1331484530668263168290072278117938725655031317451816259128204 ,
    1.2062302494209807106555860104464335480403936461999703807388699 ,
    1.2840254166877414840734205680624364583362808652814630892175078 ,
    1.3668379411737963628387567727212086721727332944308111731505954 ,
    1.4549914146182013360537936919875185083468420209644156811952411 ,
    1.5488302986341330979985519845954923375583036629258105734128605 ,
    1.6487212707001281468486507878141635716537761007101480115750791 ,
    1.7550546569602985572440470365989676887382375302457485300516132 ,
    1.8682459574322224065018356201881044531149722837225540862147661 ,
    1.9887374695822918311174773496469253668482551764105723262843917 ,
    2.1170000166126746685453698198370956101344915847024034217791335
};

double fabs(double x) {
    if (x < 0) {
        return -x;
    }
    else {
        return x;
    }
}

double exp2(double x) {
    int xInt = (int)x;
    double x_int = (double)xInt;
    double x_dec = x - x_int;

    union {
        double d;
        unsigned int i[2];
    } intExp;

    intExp.i[0] = 0;
    intExp.i[1] = (xInt + 1023) << 20;

    double exp_dec = NATURAL_LOG_2 * x_dec;

    int exp16thIndex = (int)(16.0 * exp_dec);
    double partDecExp16 = eToThe16ths_referenceTable[exp16thIndex + 12];
    exp_dec = exp_dec - ((double)(exp16thIndex)) / 16.0;

    double exp_dec_2 = exp_dec * exp_dec;
    double exp_dec_3 = exp_dec_2 * exp_dec;
    double exp_dec_4 = exp_dec_3 * exp_dec;
    double exp_dec_5 = exp_dec_4 * exp_dec;
    double exp_dec_6 = exp_dec_5 * exp_dec;
    double exp_dec_7 = exp_dec_6 * exp_dec;
    double decExp = 1.0 +
        (1.0000000000000005813768904739801428237330498589112010641013303) * exp_dec +
        (0.49999999999983123256917954535601960106310508152194805025308257) * exp_dec_2 +
        (0.16666666668564974002255329900724081715108143863509802544165922) * exp_dec_3 +
        (0.041666665571772314254795005398106852789245415877515702768895791) * exp_dec_4 +
        (0.0083333687878514162244813920507610897909388243781906785683292535) * exp_dec_5 +
        (0.0013882382228861145899774246509705912392854515454675806081607385) * exp_dec_6 +
        (0.00020471575343881367759576849243023376983966469528543746222906188) * exp_dec_7;


    return decExp * intExp.d * partDecExp16;
}

double exp(double x) {
    return exp2(LOG_2_E * x);
}

double expm1(double x) {
    if (x >= -NATURAL_LOG_2 && x <= NATURAL_LOG_2) {
        int exp16thIndex = (int)(16.0 * x);
        double partDecExp16 = eToThe16ths_referenceTable[exp16thIndex + 12];
        x = x - ((double)(exp16thIndex)) / 16.0;

        double x_2 = x * x;
        double x_3 = x_2 * x;
        double x_4 = x_3 * x;
        double x_5 = x_4 * x;
        double x_6 = x_5 * x;
        double x_7 = x_6 * x;
        double decExp = 0.0 +
            (1.0000000000000005813768904739801428237330498589112010641013303) * x +
            (0.49999999999983123256917954535601960106310508152194805025308257) * x_2 +
            (0.16666666668564974002255329900724081715108143863509802544165922) * x_3 +
            (0.041666665571772314254795005398106852789245415877515702768895791) * x_4 +
            (0.0083333687878514162244813920507610897909388243781906785683292535) * x_5 +
            (0.0013882382228861145899774246509705912392854515454675806081607385) * x_6 +
            (0.00020471575343881367759576849243023376983966469528543746222906188) * x_7;

        return decExp * partDecExp16;  
    }
    else {
        return (exp(x) - 1.0);
    }
}

double log2(double x) {
    union {
        double d;
        unsigned int i[2];
    } xCpy;

    xCpy.d = x;
    
    unsigned int topByte = xCpy.i[1];
    int intLog = (( topByte & 0x7FF00000 ) >> 20) - 1023;
    xCpy.i[1] = (topByte & 0x000FFFFF) + (1023 << 20);
    
    double x_raw = xCpy.d;

    int i;
    int intLog2 = 0;
    for (i = 0; i < 4; i++) {
        double testValue = log_refTable[i];
        if (x_raw >= testValue) {
            intLog2++;
            x_raw /= testValue;
        }
        intLog2 <<= 1;
    }

    x_raw -= 1.0;

    double x_raw_2 = x_raw * x_raw;
    double x_raw_3 = x_raw_2 * x_raw;
    double x_raw_4 = x_raw_3 * x_raw;
    double x_raw_5 = x_raw_4 * x_raw;
    double x_raw_6 = x_raw_5 * x_raw;
    double x_raw_7 = x_raw_6 * x_raw;
    double x_raw_8 = x_raw_7 * x_raw;
    double log_dec = 0.0 +
            (0.99999999999995147704417692249103946904509922655616757992340014) * x_raw +
            (-0.49999999998307599588653167637003221033156824338495091471156635) * x_raw_2 +
            (0.33333333098905741460665617330546306220710365898860199588889125) * x_raw_3 +
            (-0.2499998280832841884531528660769892404787417634091356681001292) * x_raw_4 +
            (0.19999259354660771619396926689668444874650073001297938213756922) * x_raw_5 +
            (-0.16647245207033763654872627444092570973026579310094836767871885) * x_raw_6 +
            (0.13979284894816058556984115138862700610655128388404095769009915) * x_raw_7 +
            (-0.097858084016556196356592030387085158156889291341427481578728875) * x_raw_8;



    return (log_dec + (double)intLog2 / 32.0) * LOG_2_E + (double)intLog;
}

double log(double x) {
    return log2(x) * NATURAL_LOG_2;
}

double log10(double x) {
    return log2(x) * LOG_10_2;
}

double log1p(double x) {
    if (x > -0.0625 && x < 0.0625) {
        unsigned char negative = 0;
        if (x < 0) {
            negative = 1;
            x = -x;
        }

        double x_raw_2 = x * x;
        double x_raw_3 = x_raw_2 * x;
        double x_raw_4 = x_raw_3 * x;
        double x_raw_5 = x_raw_4 * x;
        double x_raw_6 = x_raw_5 * x;
        double x_raw_7 = x_raw_6 * x;
        double x_raw_8 = x_raw_7 * x;
        double log_dec = 0.0 +
            (0.99999999999995147704417692249103946904509922655616757992340014) * x +
            (-0.49999999998307599588653167637003221033156824338495091471156635) * x_raw_2 +
            (0.33333333098905741460665617330546306220710365898860199588889125) * x_raw_3 +
            (-0.2499998280832841884531528660769892404787417634091356681001292) * x_raw_4 +
            (0.19999259354660771619396926689668444874650073001297938213756922) * x_raw_5 +
            (-0.16647245207033763654872627444092570973026579310094836767871885) * x_raw_6 +
            (0.13979284894816058556984115138862700610655128388404095769009915) * x_raw_7 +
            (-0.097858084016556196356592030387085158156889291341427481578728875) * x_raw_8;

        if (negative) {
            return -log_dec;
        }
        else {
            return log_dec;
        }
    }
    else {
        return log(x + 1.0);
    }
}

int ilogb(double x) {
    unsigned char * u8Cast = (unsigned char *)&x;

    int exponent = ( (int)(*(unsigned short *)(u8Cast + 6)) >> 4 ) & 0x7FF;

    return exponent - 1023;
}

double logb(double x) {
    return (double)ilogb(x);
}

double sqrt(double x) {
    union {
        double result;
        unsigned int resultCast[2];
    } u;

    u.result = x;

    unsigned int y = u.resultCast[1];
    unsigned int y2 = y & 0x000FFFFF;
    y2 += 0x100000 + 1506251;
    y2 *= 424;
    y2 >>= 10;
    y /= 2;
    y = y + (1023 << 20) / 2;
    unsigned char sqrtMul = 0;
    if (y & 0x00080000) {
        sqrtMul = 1;
    }
    y &= 0xFFF00000;
    y += y2 & 0xFFFFF;
    u.resultCast[1] = y;

    double r2 = u.result;

    if (sqrtMul) {
        r2 *= SQUARE_ROOT_2;
    }

    int i;
    for (i = 0; i < 3; i++) {
        r2 = ( r2 + x / r2 ) / 2;
    }

    return r2;
}

double pow(double base, double exponent) {
    return exp2(exponent * log2(base));
}

double cbrt(double x) {
    if (x >= 0) {
        return pow(x, 1.0/3.0);
    }
    else {
        return -pow(-x, 1.0/3.0);
    }
}

double hypot(double x, double y) {
    return sqrt(x*x + y*y);
}

double divideBy2piRemainder(double x) {
    //TODO -- make better -- his has all sorts of precision errors
    if (x < CONSTANT_2PI) {
        return x;
    }
    __UINT64_TYPE__ y = (int)(x / CONSTANT_2PI);
    x -= ((double)y) * CONSTANT_2PI;
    return x;
}

double sin(double x) {
    unsigned char negative = 0;

    if (x < 0) {
        x = -x;
        negative++;
    }

    if (x > CONSTANT_2PI) {
        x = divideBy2piRemainder(x);
    }

    if (x > CONSTANT_PI) {
        negative++;
        x -= CONSTANT_PI;
    }

    if (x > CONSTANT_PI / 2.0) {
        x = CONSTANT_PI - x;
    }

    double result;
    if (x > CONSTANT_PI / 4.0) {
        x = CONSTANT_PI / 2.0 - x;
        double x_2 = x * x;
        double x_3 = x_2 * x;
        double x_4 = x_3 * x;
        double x_5 = x_4 * x;
        double x_6 = x_5 * x;
        double x_7 = x_6 * x;
        double x_8 = x_7 * x;
        double x_9 = x_8 * x;
        double x_10 = x_9 * x;
        double x_11 = x_10 * x;
        double x_12 = x_11 * x;
        result = 1.0 +
                (1.6912647364569639430740473248117053810467422056910082868307514e-16) * x +
                (-0.50000000000000798695688652228588265759099165875014466140138026) * x_2 +
                (1.576934715028818689955338631142625158992289094021060997493791e-13) * x_3 +
                (0.041666666664916010924863795782699520183792558676851156454165288) * x_4 +
                (1.2308798989135382166886255389127565309498871683771122369562657e-11) * x_5 +
                (-0.0013888889470763600907425191228339173869951638149417129704934694) * x_6 +
                (1.9059824561255138701936755278799021385340064607367058873044558e-10) * x_7 +
                (2.4801150326039540313496187090966490918864754257824963633215596e-05) * x_8 +
                (6.9532643782345105099492167082873992304602300515004991547354965e-10) * x_9 +
                (-2.7631648879957109957451360031062743193647799383514823998522159e-07) * x_10 +
                (4.9347478162698486214821558440625427385316874340927633605121685e-10) * x_11 +
                (1.9246340228753454139512420669756764586363560220823995207781232e-09) * x_12;
    }
    else {
        double x_2 = x * x;
        double x_3 = x_2 * x;
        double x_4 = x_3 * x;
        double x_5 = x_4 * x;
        double x_6 = x_5 * x;
        double x_7 = x_6 * x;
        double x_8 = x_7 * x;
        double x_9 = x_8 * x;
        double x_10 = x_9 * x;
        double x_11 = x_10 * x;
        result = 0.0 +
                (1.0000000000000072475350657403182650056466434051471647457996634) * x +
                (-3.0505266004709258499981199506622885559057207795463522284961867e-13) * x_2 +
                (-0.16666666666135443431112134437926746573689896932514074429266781) * x_3 +
                (-5.1402677034166313169712250915001842596085050587107979142307923e-11) * x_4 +
                (0.0083333336438959322764479384377536983718938104430137408774059171) * x_5 +
                (-1.2391932196915021588971088929872117243528303330718575117963604e-09) * x_6 +
                (-0.00019840935261686775282851135506739547635266202369699548744916865) * x_7 +
                (-6.1141521599993555990239054516150896660614111754481114630016739e-09) * x_8 +
                (2.7630934701293954823495178711233206889101449163212298302377191e-06) * x_9 +
                (-5.420025467586329005600230583877755615760261007148566785891715e-09) * x_10 +
                (-2.3091111174159853178881552890717164340568389149278626115388049e-08) * x_11;
    }

    if (negative & 1) {
        return -result;
    }
    else {
        return result;
    }
}

double cos(double x) {
    return sin(CONSTANT_PI / 2 - x);
}

double tan(double x) {
    unsigned char negative = 0;
    if (x < 0) {
        x = -x;
        negative++;
    }

    x = divideBy2piRemainder(x);
    double result = sin(x) / cos(x);
    if (negative & 1) {
        return -result;
    }
    else {
        return result;
    }
}

double asin(double x) {
    unsigned char negative = 0;
    if (x < 0) {
        x = -x;
        negative++;
    }
    if (x > 1.0) {
        return NAN;
    }

    double b = sqrt(1 - x*x);
    double addOn;

    if (x <= SQUARE_ROOT_2 / 2) {
        if (x <= SIN_PI_OVER_8) {
            if (x <= SIN_PI_OVER_16) {
                addOn = 0;
            }
            else {
                addOn = CONSTANT_PI / 16;
                x = x * SIN_7PI_OVER_16 - b * SIN_PI_OVER_16;
            }
        }
        else {
            if (x <= SIN_3PI_OVER_16) {
                addOn = CONSTANT_PI / 8;
                x = x * SIN_3PI_OVER_8 - b * SIN_PI_OVER_8;
            }
            else {
                addOn = 3 * CONSTANT_PI / 16;
                x = x * SIN_5PI_OVER_16 - b * SIN_3PI_OVER_16;
            }
        }
    }
    else {
        if (x <= SIN_3PI_OVER_8) {
            if (x <= SIN_5PI_OVER_16) {
                x = (x - b) * SQUARE_ROOT_2 / 2;
                addOn = CONSTANT_PI / 4;
            }
            else {
                addOn = 5 * CONSTANT_PI / 16;
                x = x * SIN_3PI_OVER_16 - b * SIN_5PI_OVER_16;
            }
        }
        else {
            if (x <= SIN_7PI_OVER_16) {
                addOn = 3 * CONSTANT_PI / 8;
                x = x * SIN_PI_OVER_8 - b * SIN_3PI_OVER_8;
            }
            else {
                //printf("%.20e\n", b);
                addOn = 7 * CONSTANT_PI / 16;
                x = x * SIN_PI_OVER_16 - b * SIN_7PI_OVER_16;
            }
        }
    }

    double result;

    if (x <= CONSTANT_PI / 32.0) {
        double x_2 = x * x;
        double x_3 = x_2 * x;
        double x_4 = x_3 * x;
        double x_5 = x_4 * x;
        double x_6 = x_5 * x;
        double x_7 = x_6 * x;
        double x_8 = x_7 * x;
        double x_9 = x_8 * x;
        double x_10 = x_9 * x;
        result = 0.0 +
            (0.99999999999999924813717568541037477269449915262871627998655649) * x +
            (2.2437957416459021105919490564021046496603209516328876161112488e-13) * x_2 +
            (0.16666666663924933553279946066695783549247178633921233443195893) * x_3 +
            (1.8397233177008326875348952627946059283820538581119922737062895e-09) * x_4 +
            (0.074999923999762524994897429041190450018320618169815056461572984) * x_5 +
            (2.0373283989035922353597658032351527852172240767106911914667526e-06) * x_6 +
            (0.044606741235068942554539427406019843344360877378387784558324985) * x_7 +
            (0.00041984208740722636986034824615805276208412314333032208481720302) * x_8 +
            (0.027312574321406664959842981124658477238435723414513485616119451) * x_9 +
            (0.012719585988184251367766539391159636772747850876459613814812531) * x_10;
    }
    else {
        double x_2 = x * x;
        double x_3 = x_2 * x;
        double x_4 = x_3 * x;
        double x_5 = x_4 * x;
        double x_6 = x_5 * x;
        double x_7 = x_6 * x;
        double x_8 = x_7 * x;
        double x_9 = x_8 * x;
        double x_10 = x_9 * x;
        double x_11 = x_10 * x;
        result = -3.8027071855773326376211398858103632476253409680473167110874185e-12 +
            (1.0000000003222433189890683534567808732221511239044828343508513) * x +
            (-1.2434540116204511734237008658710516633363610087622417216066937e-08) * x_2 +
            (0.16666695541165991567329643445931922253242922880066443313888275) * x_3 +
            (-4.4901190896331240460525124849485534384193909163834318665042534e-06) * x_4 +
            (0.075049196481240586192341501298286514490634448232920325362032913) * x_5 +
            (-0.00038863517449878176782634414379750949957407091605941343879672258) * x_6 +
            (0.046865434333976530824032960043061093603594439463033594918828588) * x_7 +
            (-0.0090746934710240554159082371348954193367505153497763269667851943) * x_8 +
            (0.055839942446047950256097918358658037822667857535504428178114853) * x_9 +
            (-0.04504110139529260035566774915269226029994656668932321844577758) * x_10 +
            (0.062185299538986813900873688643648655773617294276550804749101182) * x_11;
    }

    if (negative) {
        return -(result + addOn);
    }
    else {
        return result + addOn;
    }
}

double acos(double x) {
    return CONSTANT_PI / 2 - asin(x);
}

double atan(double x) {
    double h = sqrt(1 + x*x);
    double b = 1.0 / h;
    x = x / h;

    unsigned char negative = 0;
    if (x < 0) {
        x = -x;
        negative++;
    }
    if (x > 1.0) {
        return NAN;
    }

    double addOn;

    if (x <= SQUARE_ROOT_2 / 2) {
        if (x <= SIN_PI_OVER_8) {
            if (x <= SIN_PI_OVER_16) {
                addOn = 0;
            }
            else {
                addOn = CONSTANT_PI / 16;
                x = x * SIN_7PI_OVER_16 - b * SIN_PI_OVER_16;
            }
        }
        else {
            if (x <= SIN_3PI_OVER_16) {
                addOn = CONSTANT_PI / 8;
                x = x * SIN_3PI_OVER_8 - b * SIN_PI_OVER_8;
            }
            else {
                addOn = 3 * CONSTANT_PI / 16;
                x = x * SIN_5PI_OVER_16 - b * SIN_3PI_OVER_16;
            }
        }
    }
    else {
        if (x <= SIN_3PI_OVER_8) {
            if (x <= SIN_5PI_OVER_16) {
                x = (x - b) * SQUARE_ROOT_2 / 2;
                addOn = CONSTANT_PI / 4;
            }
            else {
                addOn = 5 * CONSTANT_PI / 16;
                x = x * SIN_3PI_OVER_16 - b * SIN_5PI_OVER_16;
            }
        }
        else {
            if (x <= SIN_7PI_OVER_16) {
                addOn = 3 * CONSTANT_PI / 8;
                x = x * SIN_PI_OVER_8 - b * SIN_3PI_OVER_8;
            }
            else {
                //printf("%.20e\n", b);
                addOn = 7 * CONSTANT_PI / 16;
                x = x * SIN_PI_OVER_16 - b * SIN_7PI_OVER_16;
            }
        }
    }

    double result;

    if (x <= CONSTANT_PI / 32.0) {
        double x_2 = x * x;
        double x_3 = x_2 * x;
        double x_4 = x_3 * x;
        double x_5 = x_4 * x;
        double x_6 = x_5 * x;
        double x_7 = x_6 * x;
        double x_8 = x_7 * x;
        double x_9 = x_8 * x;
        double x_10 = x_9 * x;
        result = 0.0 +
            (0.99999999999999924813717568541037477269449915262871627998655649) * x +
            (2.2437957416459021105919490564021046496603209516328876161112488e-13) * x_2 +
            (0.16666666663924933553279946066695783549247178633921233443195893) * x_3 +
            (1.8397233177008326875348952627946059283820538581119922737062895e-09) * x_4 +
            (0.074999923999762524994897429041190450018320618169815056461572984) * x_5 +
            (2.0373283989035922353597658032351527852172240767106911914667526e-06) * x_6 +
            (0.044606741235068942554539427406019843344360877378387784558324985) * x_7 +
            (0.00041984208740722636986034824615805276208412314333032208481720302) * x_8 +
            (0.027312574321406664959842981124658477238435723414513485616119451) * x_9 +
            (0.012719585988184251367766539391159636772747850876459613814812531) * x_10;
    }
    else {
        double x_2 = x * x;
        double x_3 = x_2 * x;
        double x_4 = x_3 * x;
        double x_5 = x_4 * x;
        double x_6 = x_5 * x;
        double x_7 = x_6 * x;
        double x_8 = x_7 * x;
        double x_9 = x_8 * x;
        double x_10 = x_9 * x;
        double x_11 = x_10 * x;
        result = -3.8027071855773326376211398858103632476253409680473167110874185e-12 +
            (1.0000000003222433189890683534567808732221511239044828343508513) * x +
            (-1.2434540116204511734237008658710516633363610087622417216066937e-08) * x_2 +
            (0.16666695541165991567329643445931922253242922880066443313888275) * x_3 +
            (-4.4901190896331240460525124849485534384193909163834318665042534e-06) * x_4 +
            (0.075049196481240586192341501298286514490634448232920325362032913) * x_5 +
            (-0.00038863517449878176782634414379750949957407091605941343879672258) * x_6 +
            (0.046865434333976530824032960043061093603594439463033594918828588) * x_7 +
            (-0.0090746934710240554159082371348954193367505153497763269667851943) * x_8 +
            (0.055839942446047950256097918358658037822667857535504428178114853) * x_9 +
            (-0.04504110139529260035566774915269226029994656668932321844577758) * x_10 +
            (0.062185299538986813900873688643648655773617294276550804749101182) * x_11;
    }

    if (negative) {
        return -(result + addOn);
    }
    else {
        return result + addOn;
    }
}

double atan2(double x, double y) {
    if (y == 0) {
        if (x < 0) {
            return -CONSTANT_PI;
        }
        else {
            return CONSTANT_PI;
        }
    }
    if (x == 0) {
        if (y < 0) {
            return -CONSTANT_PI / 2;
        }
        else {
            return CONSTANT_PI / 2;
        }
    }

    if (y < 0) {
        if (x < 0) {
            return -CONSTANT_PI - atan(y / x);
        }
        else {
            return atan(y / x);
        }
    }
    else {
        if (x < 0) {
            return CONSTANT_PI - atan(y / x);
        }
        else {
            return atan(y / x);
        }
    }
}

double sinh(double x) {
    double y = exp(x);
    return (y - 1.0 / y) / 2;
}

double cosh(double x) {
    double y = exp(x);
    return (y + 1.0 / y) / 2;
}

double tanh(double x) {
    double y = exp(x);
    double z = 1.0 / y;
    return (y - z) / (y + z);
}

double asinh(double x) {
    return log(x + sqrt(x * x + 1));
}

double acosh(double x) {
    return log(x + sqrt(x * x - 1));
}

double atanh(double x) {
    return ( log(1 + x) - log(1 - x) ) / 2;
}

// STUBS!!!
double erfc(double x) {
    return NAN;
}
double erf(double x) {
    return NAN;
}
double tgamma(double x) {
    return NAN;
}
double lgamma(double x) {
    return NAN;
}

double floor(double x);
double ceil(double x) {
    if (x > -1 && x <= 0) {
        return 0;
    }
    else if (x > 0 && x <= 1) {
        return 1;
    }
    
    if (x > 0) {
        union {
            unsigned long long int i;
            double d;
        } y;
        y.d = x;
        int exponent = ( (y.i & 0x7FF0000000000000) >> 52 ) - 1023;

        if (exponent < 52) {
            if (y.i & (0xFFFFFFFFFFFFF >> exponent)) {
                y.i &= (0xFFFFFFFFFFFFFFFF << (52-exponent));
                y.d += 1.0;
                return y.d;
            }
            else {
                return x;
            }
        }
        else {
            return x;
        }
    }
    else {
        return -floor(-x);
    }
}

double floor(double x) {
    if (x >= -1 && x < 0) {
        return -1.0;
    }
    else if (x >= 0 && x < 1) {
        return 0.0;
    }

    if (x > 0) {
        union {
            unsigned long long int i;
            double d;
        } y;
        y.d = x;
        int exponent = ( (y.i & 0x7FF0000000000000) >> 52 ) - 1023;
        if (exponent < 52) {
            if (y.i & (0xFFFFFFFFFFFFF >> exponent)) {
                y.i &= (0xFFFFFFFFFFFFFFFF << (52-exponent));
                return y.d;
            }
            else {
                return x;
            }
        }
        else {
            return x;
        }
    }
    else {
        return -ceil(-x);
    }
}

double trunc(double x) {
    if (x >= 0) {
        return floor(x);
    }
    else {
        return -floor(-x);
    }
}

double round(double x) {
    if (x >= 0) {
        double y = floor(x);
        if (y - x >= 0.5) {
            y += 1.0;
        }
        return y;
    }
    else {
        double y = floor(-x);
        if (y - x >= 0.5) {
            y += 1.0;
        }
        return -y;
    }
}

double nearbyint(double x) {
    return round(x);
}

double rint(double x) {
    return round(x);
}

double frexp( double x, int* exp ) {
    if (x == 0) {
        *exp = 0;
        return 0;
    }

    union {
        double d;
        unsigned int i[2];
    } y;
    y.d = x;

    unsigned int exponent = ( (y.i[1] & 0x7FF00000) >> 20 ) + 1023;
    y.i[1] &= 0x000FFFFFF;
    y.i[1] += (1023 << 20);

    y.d /= 2;
    exponent++;

    *exp = exponent;
    return y.d;
}

double ldexp( double x, int exp ) {
    if (x == 0) {
        return 0;
    }

    union {
        double d;
        unsigned int i[2];
    } y;
    y.d = x;

    y.i[2] += (exp << 20);
    return y.d;
}

double modf( double x, double * iptr ) {
    double x_int = trunc(x);
    double x_frac = x - x_int;

    *iptr = x_int;
    return x_frac;
}

double scalbn( double x, int exp ) {
    return ldexp(x, exp);
}
