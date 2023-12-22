/////////////////////////////////////////////   PREFACE   /////////////////////////////////////////////////////

/* This project will be heavily commented to help anyone who uses any of this to understand what I am doing
and honestly to help me understand or remember what I am doing. Keep in mind that I am NOT a coder. I have
absolutely zero experience, I am just a guy who likes to tinker and learn new stuff who is teaching myself to 
code through documentation and experimentation. That being said the odds of my code being poorly written,
wrong reasons listed for doing what i did etc. is probably very high so take it all with a grain of salt.
Project is written for an M5Stack Core S3 device and is a simple GUI for an electric skateboard I built.*/

/////////////////////////////////////////////   INCLUDES   /////////////////////////////////////////////////////

#include <Arduino.h> // Necessary for critical functions and definitions (such as setup() and loop())

#include <SD.h> /* Necessary to write / read to the SD card, although I dont have codd for that in here yet.
                required to be #included before M5Unified and LVGL or can cause issues */

#include <M5Unified.h> /* M5Unified takes the place of device specific libraries. Because of this, it should be
                       the only "M5" library you #include as having multiple can cause compiling errors due to
                       similar namespaces being used. Also it has a dependency of the M5GFX library (Does not use
                       Bodmers TFT_eSPI). If you are using Arduino IDE then you will need to include M5GFX. If you
                       are using PlatformIO, then it will automatically be included when you include "M5Unified"*/

#include <lvgl.h> // LVGL is a powerful graphics library which I am using to make the GUI.

#include <Wire.h> // Used to initialize and control devices on I2C bus

#include <WiFi.h> // Used to create a connection to your wifi

/////////////////////////////////////////////   DECLARATIONS   /////////////////////////////////////////////////////

const char *ssid = "CSL12ProMax";     // Your Wi-Fi SSID
const char *password = "seahawks!"; // Your Wi-Fi password
int wifiTimeout = 10000; // 10 seconds timeout
unsigned long wifiStartTime;

// These variables are for holding the touch position data for the X and Y coordinates // 
int touchedX = 0; 
int touchedY = 0;

// These are for the touchscreen dimensions. It makes it easier to do some functions using variables vice numbers // 
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

// This part establishes a buffer for our display using LVGL
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];


// These are LVGL object declarations. Like a placeholder telling the computer "you are gonna use them soon so
// learn their names now so you arent confused later when I tell you to use them".
lv_obj_t *butbat;
lv_obj_t *butspd;
lv_obj_t *buttrp;
lv_obj_t *butnav;
lv_obj_t *butback;
lv_obj_t *scrbat;
lv_obj_t *img_bat;
lv_obj_t *scrspd;
lv_obj_t *img_spd;
lv_obj_t *scrtrp;
lv_obj_t *img_trp;
lv_obj_t *scrnav;
lv_obj_t *img_nav;
lv_obj_t *lbl_m5bat;



// These are LVGL image declarations. If you are using images in C arrays, you will need these.
LV_IMG_DECLARE(bg_spd);
LV_IMG_DECLARE(bg_bat);
LV_IMG_DECLARE(bg_home);
LV_IMG_DECLARE(bg_nav);
LV_IMG_DECLARE(bg_trip);
LV_IMG_DECLARE(but_back);
LV_IMG_DECLARE(but_bat);
LV_IMG_DECLARE(but_nav);
LV_IMG_DECLARE(but_spd);
LV_IMG_DECLARE(but_trp);

// These are functions I wrote to control the loading of different screens in the GUI
void load_scrhome();
void load_scrbat(); 
void load_scrnav();
void load_scrtrp();
void load_scrspd();


/////////////////////////////////////////////   FUNCTIONS   /////////////////////////////////////////////////////

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  M5.Display.setCursor(45, 120);
  M5.Display.setTextSize(2);
  M5.Display.print("Connecting to Wi-Fi");

  wifiStartTime = millis(); // Record the start time

  while (WiFi.status() != WL_CONNECTED && (millis() - wifiStartTime) < wifiTimeout) {
    delay(1000);
    Serial.print(".");
    M5.Display.setCursor(45,140);
    M5.Display.print(".");
    delay(100);
    M5.Display.setCursor(47,140);
    M5.Display.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi");
    M5.Display.clear();
    M5.Display.setCursor(100, 120);
    M5.Display.setTextSize(2);
    M5.Display.print("Connected!");
    delay(2000);
  } else {
    Serial.println("\nWi-Fi connection timed out");
    M5.Display.clear();
    M5.Display.setCursor(45, 120);
    M5.Display.setTextSize(2);
    M5.Display.print("Connection Failed");
    delay(2000);
    exit; // Handle the case when Wi-Fi connection fails (e.g., continue without Wi-Fi)
  }
}


// This is a callback function. Tells program what to do when battery button is pressed.
void cb_butbat(lv_event_t *event) {
    Serial.println("Battery button clicked!"); // Prints to the serial monitor, useful for troublshooting.
    if (!scrbat) { // Create the battery screen only if it doesn't exist
        M5.Speaker.setVolume(25); // Set the speaker volume to 25 (out of 255)
        M5.Speaker.tone(4000, 5, 1, false); // Play a 4000Hz tone for 5  milliseconds, channel 1, dont interupt other tones.
        M5.Speaker.tone(2000, 5, 2, false); // Play a 2000Hz tone for 5 milliseconds, channel 2, dont interupt other tones.
        load_scrbat(); // Load the battery screen
    }
}

void cb_butspd(lv_event_t *event) {
    Serial.println("Speed button clicked!");
    if (!scrspd) {// 
        M5.Speaker.setVolume(25);
        M5.Speaker.tone(4000, 5, 1, false); // Makes a beep tone when button is pressed
        M5.Speaker.tone(2000, 5, 1, false); // Makes a beep tone when button is pressed
        load_scrspd(); 
    }
}

void cb_buttrp(lv_event_t *event) {
    Serial.println("Trip button clicked!");
    if (!scrtrp) {
        M5.Speaker.setVolume(25);
        M5.Speaker.tone(4000, 5, 1, false); // Makes a beep tone when button is pressed
        M5.Speaker.tone(2000, 5, 1, false); // Makes a beep tone when button is pressed
        load_scrtrp(); 
    }
}

void cb_butnav(lv_event_t *event) {
    Serial.println("Navigation button clicked!");
    if (!scrnav) {
        M5.Speaker.setVolume(25);
        M5.Speaker.tone(4000, 5, 1, false); // Makes a beep tone when button is pressed
        M5.Speaker.tone(2000, 5, 1, false); // Makes a beep tone when button is pressed
        load_scrnav();
    }
}

void cb_butback(lv_event_t *event) {
    lv_obj_t *current_screen = lv_scr_act();

    if (current_screen != NULL) {
        M5.Speaker.setVolume(25);
        M5.Speaker.tone(4000, 5, 1, false); // Makes a beep tone when button is pressed
        M5.Speaker.tone(2000, 5, 1, false); // Makes a beep tone when button is pressed
        lv_obj_clean(current_screen);

        // Reset screen references if needed
        if (current_screen == scrbat) {
            scrbat = NULL;
        } else if (current_screen == scrspd) {
            scrspd = NULL;
        } else if (current_screen == scrtrp) {
            scrtrp = NULL;
        } else if (current_screen == scrnav) {
            scrnav = NULL;
        }

        load_scrhome(); // Load the home screen
    }
}


// Function to check if a point is within the area of a button
bool isWithinButtonArea(lv_obj_t *button, int x, int y) {
    lv_area_t button_area;
    lv_obj_get_coords(button, &button_area);

    return (x >= button_area.x1 && x <= button_area.x2 &&
            y >= button_area.y1 && y <= button_area.y2);
}
 // Function to handle touch inputs and output to other functions for use
void tch() {
    lgfx::touch_point_t tp[3];
    int nums = M5.Display.getTouchRaw(tp, 3);
    if (nums) {
        touchedX = tp[0].x;
        touchedY = tp[0].y;

        Serial.printf("Raw X:%03d Y:%03d\n", tp[0].x, tp[0].y); // Good for troublshooting issues

        if (isWithinButtonArea(butbat, touchedX, touchedY)) {
            Serial.println("Battery button clicked!");
            cb_butbat(NULL);
        } else if (isWithinButtonArea(butspd, touchedX, touchedY)) {
            Serial.println("Speed button clicked!");
            cb_butspd(NULL);
        } else if (isWithinButtonArea(buttrp, touchedX, touchedY)) {
            Serial.println("Trip button clicked!");
            cb_buttrp(NULL);
        } else if (isWithinButtonArea(butnav, touchedX, touchedY)) {
            Serial.println("Navigation button clicked!");
            cb_butnav(NULL);
        } else if (isWithinButtonArea(butback, touchedX, touchedY)) {
            Serial.println("Back button clicked!");
            cb_butback(NULL);
        }
    }
    vTaskDelay(1); // Tiny delay to give the machine time to think and not trip itself up
}

// This function controls the display flushing. Its what takes the data stored in your buffer and shows
// it on the physical screen.
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    M5.Lcd.startWrite();
    M5.Lcd.setAddrWindow(area->x1, area->y1, w, h);
    M5.Lcd.pushColors((uint16_t *)color_p, w * h, true); // I think this is decepracated now but it worked for me
    M5.Lcd.endWrite();
    lv_disp_flush_ready(disp_drv);
}

// My function to load the home screen
void load_scrhome() {
    lv_obj_t *scrhome = lv_obj_create(NULL); // LVGL make a screen named "scrhome".
    lv_obj_t *img_home = lv_img_create(scrhome); // Make image container named "img_home"
    lv_img_set_src(img_home, &bg_home); // Make the image container fill from here
    lv_obj_align(img_home, LV_ALIGN_TOP_LEFT, 0, 0); // Move the container here
    lv_obj_set_height(img_home, 240); // Make the container this high
    lv_obj_set_width(img_home, 320); // Make the container this wide

    butbat = lv_imgbtn_create(scrhome); // make a button called "butbat" on the "scrhome" screen
    butspd = lv_imgbtn_create(scrhome); // You get the idea
    buttrp = lv_imgbtn_create(scrhome);
    butnav = lv_imgbtn_create(scrhome);

    lv_obj_set_pos(butbat, 20, 90); // Sets the positions of the buttons and label
    lv_obj_set_pos(butspd, 95, 90);
    lv_obj_set_pos(buttrp, 165, 90);
    lv_obj_set_pos(butnav, 240, 90);

    lv_obj_set_height(butbat, 60); // Sets the Height and width of buttons and label
    lv_obj_set_width(butbat, 60);
    lv_obj_set_height(butspd, 60);
    lv_obj_set_width(butspd, 60);
    lv_obj_set_height(buttrp, 60);
    lv_obj_set_width(buttrp, 60);
    lv_obj_set_height(butnav, 60);
    lv_obj_set_width(butnav, 60);
   
    lv_img_set_src(butbat, &but_bat); // Sets the image source for the buttons (I drew my own)
    lv_img_set_src(butspd, &but_spd);
    lv_img_set_src(buttrp, &but_trp);
    lv_img_set_src(butnav, &but_nav);
    
    lv_obj_add_event_cb(butbat, cb_butbat, LV_EVENT_CLICKED, NULL); // Add the callbacks to the 
    lv_obj_add_event_cb(butspd, cb_butspd, LV_EVENT_CLICKED, NULL); // buttons so that they 
    lv_obj_add_event_cb(buttrp, cb_buttrp, LV_EVENT_CLICKED, NULL); // actually do something.
    lv_obj_add_event_cb(butnav, cb_butnav, LV_EVENT_CLICKED, NULL);

    lv_scr_load(scrhome); // Finally load everything we just made onto the screen.
}

// This function loads the battery screen. Same type of stuff thats in the function above.
void load_scrbat() {
    scrbat = lv_obj_create(NULL);
    img_bat = lv_img_create(scrbat);
    butback = lv_imgbtn_create(scrbat);
    lv_img_set_src(img_bat, &bg_bat);
    lv_img_set_src(butback, &but_back);
    lv_obj_align(img_bat, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_align(butback, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_height(img_bat, 240);
    lv_obj_set_width(img_bat, 320);
    lv_obj_set_height(butback, 60);
    lv_obj_set_width(butback, 60);
    lv_scr_load_anim(scrbat, LV_SCR_LOAD_ANIM_FADE_IN, 150, 0, true);
}

// And this one loads the speed screen
void load_scrspd() {
    scrspd = lv_obj_create(NULL);
    img_spd = lv_img_create(scrspd);
    butback = lv_imgbtn_create(scrspd);
    lv_img_set_src(img_spd, &bg_spd);
    lv_img_set_src(butback, &but_back);
    lv_obj_align(img_spd, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_align(butback, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_height(img_spd, 240);
    lv_obj_set_width(img_spd, 320);
    lv_obj_set_height(butback, 60);
    lv_obj_set_width(butback, 60);
    lv_scr_load_anim(scrspd, LV_SCR_LOAD_ANIM_FADE_IN, 150, 0, true);
}

// This one loads the trip screen
void load_scrtrp() {
    scrtrp = lv_obj_create(NULL);
    img_trp = lv_img_create(scrtrp);
    butback = lv_imgbtn_create(scrtrp);
    lv_img_set_src(img_trp, &bg_trip);
    lv_img_set_src(butback, &but_back);
    lv_obj_align(img_trp, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_align(butback, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_height(img_trp, 240);
    lv_obj_set_width(img_trp, 320);
    lv_obj_set_height(butback, 60);
    lv_obj_set_width(butback, 60);
    lv_scr_load_anim(scrtrp, LV_SCR_LOAD_ANIM_FADE_IN, 150, 0, true);
}

//This one loads the Navigation screen
void load_scrnav() {
    scrnav = lv_obj_create(NULL);
    img_nav = lv_img_create(scrnav);
    butback = lv_imgbtn_create(scrnav);
    lv_img_set_src(img_nav, &bg_nav);
    lv_img_set_src(butback, &but_back);
    lv_obj_align(img_nav, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_align(butback, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_height(img_nav, 240);
    lv_obj_set_width(img_nav, 320);
    lv_obj_set_height(butback, 60);
    lv_obj_set_width(butback, 60);
    lv_scr_load_anim(scrnav, LV_SCR_LOAD_ANIM_FADE_IN, 150, 0, true);



}



/////////////////////////////////////////////   SETUP   /////////////////////////////////////////////////////

// Code in the setup will run only once, from top to bottom.
void setup() {
    M5.begin(); // Initializes the M5 Stack unit. There is a better way to do this by passing a function
                // but if you dont know, just do it this way. (I was lazy, had it the right way and accidently deleted)
    Serial.begin(115200); // Initialize the serial port to output your data from the "serial print" statements.
    M5.Speaker.setVolume(25); // Kinda self explanatory
    connectToWiFi(); // Connect to Wi-Fi
    lv_init(); // Initialize LVGL. Make sure you do this after initializing the M5 device itself.
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10); // Initialize the buffer we made
    lv_disp_drv_t *disp_drv = (lv_disp_drv_t *)malloc(sizeof(lv_disp_drv_t)); // Allocate memory for the display use
    if (!disp_drv) { // Error handling for failure to init the display
        return;
    }
    lv_disp_drv_init(disp_drv); // Initialize the display drivers
    disp_drv->hor_res = screenWidth; // Tell it what resolution to output, thats why we made these variables
    disp_drv->ver_res = screenHeight;
    disp_drv->flush_cb = my_disp_flush; // tell the driver which function we created to handle the flush requirements
    disp_drv->draw_buf = &draw_buf; // Same for buffer
    lv_disp_drv_register(disp_drv); // Finally register it all witht he system

    int sdaPin = 12; // Creating variables to pass arguments to the Wire.begin() function
    int sclPin = 11; // Pin 11 and 12 control most items on the CoreS3 seems like
    Wire.begin(sdaPin, sclPin);

    load_scrhome(); // Load the initial home screen
   
}

/////////////////////////////////////////////   MAIN LOOP   /////////////////////////////////////////////////////

void loop() {
    M5.update(); // Function to update the M5 Stack regularly
    tch(); // Custom function to monitor and handle touch input
    lv_task_handler(); // Function that handles LVGL tasks
    
}
