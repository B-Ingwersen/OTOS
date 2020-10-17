#ifndef OTOS_CORE___C_STANDARD_LIBRARY___LOCALE_H
#define OTOS_CORE___C_STANDARD_LIBRARY___LOCALE_H

#define LC_ALL 1	        //selects the entire C locale
#define LC_COLLATE 2	    //selects the collation category of the C locale
#define LC_CTYPE 3	    //selects the character classification category of the C locale
#define LC_MONETARY 4	//selects the monetary formatting category of the C locale
#define LC_NUMERIC 5	    //selects the numeric formatting category of the C locale
#define LC_TIME 6

struct lconv {
   char * decimal_point;
   char * thousands_sep;
   char * grouping;	
   char * int_curr_symbol;
   char * currency_symbol;
   char * mon_decimal_point;
   char * mon_thousands_sep;
   char * mon_grouping;
   char * positive_sign;
   char * negative_sign;
   char int_frac_digits;
   char frac_digits;
   char p_cs_precedes;
   char p_sep_by_space;
   char n_cs_precedes;
   char n_sep_by_space;
   char p_sign_posn;
   char n_sign_posn;
};

extern const struct lconv cStandardLibrary_GlOBAL_VARAIABLE_Lconv_Table;

char * setlocale( int category, const char * locale);

struct lconv * localeconv();

#endif
