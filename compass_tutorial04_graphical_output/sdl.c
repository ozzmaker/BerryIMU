#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>

#define TEXTANGLE 90.0
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320
#define SCREEN_BPP 16




SDL_Surface* compatible_image = NULL;
SDL_Surface* screen = NULL;
SDL_Surface* rotation = NULL;
SDL_Surface* outerRing = NULL;
SDL_Surface* compassNeedle = NULL;
SDL_Surface* compatibleOuterRing = NULL;
SDL_Surface* compatibleCompassNeedle = NULL;
SDL_Surface* textSurface = NULL;
SDL_Surface* currentDegressRotated = NULL;
TTF_Font *font;



void startSDL()
{
        //fb1 = small TFT.   fb0 = HDMI/RCA output
        putenv("SDL_FBDEV=/dev/fb1");



        //Initialize  SDL and disable mouse
        SDL_Init(SDL_INIT_VIDEO);
        SDL_ShowCursor(SDL_DISABLE);

        //Get information about the current video device.  E.g. resolution and bits per pixal
        const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();

        //Setup a Video mode.
        screen = SDL_SetVideoMode(videoInfo->current_w, videoInfo->current_h, videoInfo->vfmt->BitsPerPixel, SDL_SWSURFACE );
        if ( screen == NULL ) {
                fprintf(stderr, "Unable to setvideo: %s\n", SDL_GetError());
                exit(1);
        }

        TTF_Init();

        SDL_Color colorWhite = {255,255,255,0};
        font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 42);


        //Load compass images
        outerRing =  IMG_Load("OuterRing.png");
        compassNeedle =  IMG_Load("CompassNeedle.png");
        if (outerRing == NULL || compassNeedle == NULL) printf("error\n");

        //Convert Alpha to a compatible format
        compatibleOuterRing = SDL_DisplayFormatAlpha( outerRing );
        compatibleCompassNeedle = SDL_DisplayFormatAlpha( compassNeedle);



}

void closeSDL()
{
        SDL_FreeSurface(compatible_image);
        SDL_FreeSurface(screen);
        SDL_FreeSurface(rotation);
        SDL_FreeSurface(outerRing);
        SDL_FreeSurface(compassNeedle);
        SDL_FreeSurface(compatibleOuterRing);
        SDL_FreeSurface(compatibleCompassNeedle);
        SDL_FreeSurface(textSurface);
        SDL_FreeSurface(currentDegressRotated);


	TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
}


int graphics( float heading)
{

        SDL_Rect outerRingposition;
        SDL_Rect compassNeedlePosition;


        SDL_Rect topLine1;
        topLine1.x = 20;
        topLine1.y = 100;
        topLine1.w = 0;
        topLine1.h = 0;


        SDL_Color colorWhite = {255,255,255,0};


        char headingString[8] = "";

        //Clear previous image
        SDL_FillRect(screen, NULL, 0x000000);

        //Convert heading value which is a float to a string
        snprintf(headingString, 7, "%7.3f", heading);

        textSurface = TTF_RenderText_Solid(font, headingString, colorWhite);

        //Rotate the heading
        currentDegressRotated =  rotozoomSurface(textSurface, TEXTANGLE, 1.0, 0);

        //Position the outer ring in the center of display
        outerRingposition.x = (SCREEN_WIDTH - compatibleOuterRing->w)/2;
        outerRingposition.y = (SCREEN_HEIGHT - compatibleOuterRing->h)/2;

        //Position compass needle
        compassNeedlePosition.x = (SCREEN_WIDTH - compatibleCompassNeedle->w)/2;
        compassNeedlePosition.y = 20;

        //Rotate needle based on angle. Add 90 degrees for correction.
        rotation = rotozoomSurface(compatibleCompassNeedle, heading+90, 1.0, 0);
        if (rotation == NULL) printf("error rotating needle\n");

        //recenter pivote for rotation
        compassNeedlePosition.x -= rotation->w/2-compatibleCompassNeedle->w/2;
        compassNeedlePosition.y -= rotation->h/2-compatibleCompassNeedle->h/2;

        // put the image on the screen surface
        SDL_BlitSurface(currentDegressRotated, NULL, screen, &topLine1);
        SDL_BlitSurface(compatibleOuterRing, NULL, screen, &outerRingposition);
        SDL_BlitSurface(rotation, NULL, screen, &compassNeedlePosition);

        // send the screen surface to be displayed
        SDL_Flip(screen);

        SDL_FreeSurface(currentDegressRotated);
        SDL_FreeSurface(textSurface);
        SDL_FreeSurface(screen);
        SDL_FreeSurface(rotation);

        return 0;
}
