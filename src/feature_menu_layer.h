#include <pebble.h>
#include <pebble_process_info.h>
#undef APP_LOG
#define APP_LOG(...)
	
#ifndef PBL_PLATFORM_BASALT
#define gbitmap_get_bounds(bitmap) bitmap->bounds;
#endif
	
typedef enum ITEMS {
	TOTAL					= 0,
 	COCAINE 			= 1,
 	HEROINE 			= 2,
 	ACID					= 3,
 	WEED					= 4,
 	SPEED					= 5,
	LUDES					= 6,
} TRENCHCOAT_ITEMS;

enum LOCATIONS {
		NEVERMIND			= 0,
		BRONX       	= 1,
		GHETTO       	= 2,
		CENTRAL_PARK 	= 3,
		MANHATTEN    	= 4,
		CONEY_ISLAND 	= 5,
		BROOKLYN     	= 6
};

enum KEYS {
	VERSION					= 0,
	VIBRATE					= 1,
	INVERT					= 2,
	LIGHT 					= 3,
	DAYS						= 4
};

#define HIGH_SCORE_KEY							0
#define SETTINGS_DATA_KEY						248
#define PLAYER_SIZE_KEY							5
#define PLAYER_DATA_KEY							10
#define NUM_MENU_ICONS 							9
#define MENU_CELL_BASIC_HT					17
#define	MENU_CELL_BASIC_HEADER_HT		26
#define SUBTITLED_MENU_HEADER_HT	 	44
#define NUM_MENU_SECTIONS						1
#define NUM_HOME_MENU_ITEMS 				7
#define NUM_PRICES_MENU_ITEMS				7
#define NUM_SELL_MENU_ITEMS					7
#define BASIC_ITEM_LENGTH						17
#define MAX_ITEM_LENGTH							63
#define SHORT_MESSAGE_DELAY					2000
#define LONG_MESSAGE_DELAY					5000
#define PUNISHMENT_DELAY						10000

InverterLayer *inverter_layer;
MenuLayer 		*home_menu_layer;
MenuIndex 		*p_NumWindowContext = NULL;
GBitmap 			*menu_icons[NUM_MENU_ICONS];
GBitmap 			*game_icon = NULL;

// Get app version info
extern const PebbleProcessInfo __pbl_app_info;

// In-Game Variables
int				value, X, Y, Score;
bool			num_window_is_visible;
short 		current_icon;
short			menu_number = 0;
GFont 		header_font;
GFont 		cell_font;
GFont 		subtitle_font;
GFont			confirm_font;

// Basic Menu item type
typedef struct {
	const char *title;
	const char *subtitle;
} BasicItem;

const short NUM_DAYS[4] = {30, 15, 45, 60};

// Monetary unit designations
const char postfix[4] = {'\0','K', 'M', 'B'};

// Menu Header Heights
const short menu_header_heights[10] =
{
	MENU_CELL_BASIC_HEADER_HT			,
	MENU_CELL_BASIC_HEADER_HT			, 
	SUBTITLED_MENU_HEADER_HT			,
	MENU_CELL_BASIC_HEADER_HT			, 
	MENU_CELL_BASIC_HEADER_HT			, 
	SUBTITLED_MENU_HEADER_HT			,
	MENU_CELL_BASIC_HEADER_HT * 3	,
	MENU_CELL_BASIC_HEADER_HT * 3 ,
	SUBTITLED_MENU_HEADER_HT + 19	,
	MENU_CELL_BASIC_HEADER_HT * 4
};

// Home Menu
BasicItem menu_items[8]	=
{
	{"BACK",NULL},
	{"PRICES",NULL},
	{"TRENCHCOAT",NULL},
	{"BUY",NULL},
	{"SELL",NULL},
	{"SUBWAY",NULL},
	{"LOAN SHARK",NULL},
	{"BANK",NULL}
};

// Trenchcoat Menu
const char* trenchcoat_items[6] =
{
		"BACK",
		"AMMO        %u ",
		"DAMAGE      %i ",
		"DRUGS       %u ",
		"GUNS        %i ",
		"FREESPACE   %u "
};

// Subway Menu
const char* locations[7] =
{
		"",
		"THE BRONX",
		"THE GHETTO",
		"CENTRAL PARK",
		"MANHATTEN",
		"CONEY ISLAND",
		"BROOKLYN"
};

// Loan Menu
const char* loan_menu[3] =
{
		"",
		"PAY   ",
		"BORROW"
};

// Bank Menu
const char* bank_menu[3] =
{
		"",
		"WITHDRAW",
		"DEPOSIT"
};

// Chased Menu
const char* chased_menu[7] =
{
	"%i PIG",
	"IN PERSUIT!!",
	"AMMO         %i",
	"GUNS         %i",
	"DAMAGE %i OF 50",
	"RUN",
	"FIGHT"
};

// Confirm Menu
const char* confirm_menu[2] =
{
	"NO",
	"YES"
};

typedef struct {
	int 			Price;
	int 					Quantity;
	const char 		*Name;
} DRUGS;

typedef struct {
	int 			Capacity;
	int 			Damage;
	int 			Quantity;
	int 			Ammo;
	int 			Price;
	char 					*Name;
} GUNS;

typedef struct {
	int 			Price;
	int 			Capacity;
	int 			Freespace;
	GUNS 			Guns[4];
	DRUGS 		Drug[7];				
} INVENTORY;

typedef struct {
	int 			Balance;
	int 			Cash;
	int 			Debt;
} FINANCIAL_DATA;

typedef struct {
	int 			Cops;
	int				CurrentCity;
	int 			Damage;
	int 			Day;
	int				Dice;
	int				MenuNumber;
	int 			Health;
	FINANCIAL_DATA Money;
	INVENTORY Trenchcoat;
} PLAYER_DATA;

PLAYER_DATA Player;

typedef void						(*MenuCallback)(MenuIndex *);
// In-Game functions
void 	Intro							(MenuIndex *);
void 	Being_Shot				(MenuIndex *);
void 	Buy_Trenchcoat		(MenuIndex *);
void 	Buy_Gun						(MenuIndex *);
void 	Cop_187						(MenuIndex *);
void 	Doctor						(MenuIndex *);
void 	Event_Generator		(MenuIndex *);
void 	Exit							(MenuIndex *);
void 	Game_Over					(MenuIndex *);
void	Load_Game					(MenuIndex *);
void 	Smoke_It					(MenuIndex *);
void 	UpdateFreespace		(MenuIndex *);
void 	Play_Again				(MenuIndex *);
void 	BuyDrugs					(int, MenuIndex *);
void 	SellDrugs					(int, MenuIndex *);
void 	Save_Game					(void);
void  set_invert_layer	(void);
void 	Show_Instructions	(void *);
void 	show_number_window_layer(void *);
void 	hide_number_window_layer(void);
void 	num_selected_callback(struct NumberWindow *, void *);

MenuCallback p_MenuCallbackContext[2] = {NULL, NULL};

// Pebble wrapper conditional functions
void 	Num_Input(char *, int, int, int, int, MenuIndex *);

// App specific number functions
int 	LOG10(int val);
int 	EXP(int val);
void	float2string(char *, double, short);
void 	floatstrcat(char *, double, int);

// Menu Header Draw function for Title only
void 	menu_header_simple_draw(GContext *, const Layer *, const char *);

// Menu Header Draw function for Icon and Title
void 	menu_header_simple_icon_draw(GContext *, const Layer *, const char *, const GBitmap *);

// Menu Header Draw function for Icon, Title, and Subtitle
void 	menu_header_draw(GContext *, const Layer *, const char *, const char *, const GBitmap *);

// Menu Header Draw function for titles (multiple-lines)
void 	menu_header_long_draw(GContext *, const Layer *, const char *);

// Menu Row Draw function for Title only
void 	menu_cell_simple_draw(GContext *, const Layer *, const char *);

// Menu Row Draw function for Title only (Centered)
void 	menu_cell_simple_centered_draw(GContext *, const Layer *, const char *);

// Menu Row Draw function for Icon and Title
void 	menu_cell_simple_icon_draw(GContext *, const Layer *, const char *, const GBitmap *);

//! Menu row drawing function to draw a basic cell with the title, subtitle, and icon. 
//! Call this function inside the `.draw_row` callback implementation, see \ref MenuLayerCallbacks.
//! @param ctx The destination graphics context
//! @param cell_layer The layer of the cell to draw
//! @param title Draws a title in larger text (18 points, Lucida Console font).
//! @param subtitle Draws a subtitle in smaller text (14 points, Lucida Console font).
//! @param icon Draws an icon to the left of the text.
void 	menu_cell_draw(GContext *, const Layer *, const char *, const char *, const GBitmap *);

char	*format;
char	*string;
char 	*confirm_header;
char	*version;