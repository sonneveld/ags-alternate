
#include "allegro.h"

#include "ac_screen.h"

#include "ac.h"
#include "ac_context.h"
#include "acgfx.h"
#include "ali3d.h"


/* *** SCRIPT SYMBOL: [Screen] TintScreen *** */
void TintScreen(int red, int grn, int blu) {
  if ((red < 0) || (grn < 0) || (blu < 0) || (red > 100) || (grn > 100) || (blu > 100))
    quit("!TintScreen: RGB values must be 0-100");

  invalidate_screen();

  if ((red == 0) && (grn == 0) && (blu == 0)) {
    play.screen_tint = -1;
    return;
  }
  red = (red * 25) / 10;
  grn = (grn * 25) / 10;
  blu = (blu * 25) / 10;
  play.screen_tint = red + (grn << 8) + (blu << 16);
}


/* *** SCRIPT SYMBOL: [Screen] FadeOut *** */
void my_fade_out(int spdd) {
  EndSkippingUntilCharStops();

  if (play.fast_forward)
    return;

  if (play.screen_is_faded_out == 0)
    gfxDriver->FadeOut(spdd, play.fade_to_red, play.fade_to_green, play.fade_to_blue);

  if (game.color_depth > 1)
    play.screen_is_faded_out = 1;
}

void my_fade_in(ALW_PALETTE p, int speed) {
  if (game.color_depth > 1) {
    alw_set_palette (p);

    play.screen_is_faded_out = 0;

    if (play.no_hicolor_fadein) {
      return;
    }
  }

  gfxDriver->FadeIn(speed, p, play.fade_to_red, play.fade_to_green, play.fade_to_blue);
}


/* *** SCRIPT SYMBOL: [Screen] SetScreenTransition *** */
void SetScreenTransition(int newtrans) {
  if ((newtrans < 0) || (newtrans > FADE_LAST))
    quit("!SetScreenTransition: invalid transition type");

  play.fade_effect = newtrans;

  DEBUG_CONSOLE("Screen transition changed");
}

/* *** SCRIPT SYMBOL: [Screen] SetNextScreenTransition *** */
void SetNextScreenTransition(int newtrans) {
  if ((newtrans < 0) || (newtrans > FADE_LAST))
    quit("!SetNextScreenTransition: invalid transition type");

  play.next_screen_transition = newtrans;

  DEBUG_CONSOLE("SetNextScreenTransition engaged");
}

/* *** SCRIPT SYMBOL: [Screen] SetFadeColor *** */
void SetFadeColor(int red, int green, int blue) {
  if ((red < 0) || (red > 255) || (green < 0) || (green > 255) ||
      (blue < 0) || (blue > 255))
    quit("!SetFadeColor: Red, Green and Blue must be 0-255");

  play.fade_to_red = red;
  play.fade_to_green = green;
  play.fade_to_blue = blue;
}



/* *** SCRIPT SYMBOL: [Screen] FlipScreen *** */
void FlipScreen(int amount) {
  if ((amount<0) | (amount>3)) quit("!FlipScreen: invalid argument (0-3)");
  play.screen_flipped=amount;
}


/* *** SCRIPT SYMBOL: [Screen] ShakeScreen *** */
void ShakeScreen(int severe) {
  EndSkippingUntilCharStops();

  if (play.fast_forward)
    return;

  int hh;
  block oldsc=abuf;
  severe = multiply_up_coordinate(severe);

  if (gfxDriver->RequiresFullRedrawEachFrame())
  {
    play.shakesc_length = 10;
    play.shakesc_delay = 2;
    play.shakesc_amount = severe;
    play.mouse_cursor_hidden++;

    for (hh = 0; hh < 40; hh++) {
      loopcounter++;
      platform->Delay(50);

      render_graphics();

      update_polled_stuff();
    }

    play.mouse_cursor_hidden--;
    clear_letterbox_borders();
    play.shakesc_length = 0;
  }
  else
  {
    block tty = alw_create_bitmap(scrnwid, scrnhit);
    gfxDriver->GetCopyOfScreenIntoBitmap(tty);
    for (hh=0;hh<40;hh++) {
      platform->Delay(50);

      if (hh % 2 == 0) 
        render_to_screen(tty, 0, 0);
      else
        render_to_screen(tty, 0, severe);

      update_polled_stuff();
    }
    clear_letterbox_borders();
    render_to_screen(tty, 0, 0);
    wfreeblock(tty);
  }

  abuf=oldsc;
}

/* *** SCRIPT SYMBOL: [Screen] ShakeScreenBackground *** */
void ShakeScreenBackground (int delay, int amount, int length) {
  if (delay < 2) 
    quit("!ShakeScreenBackground: invalid delay parameter");

  if (amount < play.shakesc_amount)
  {
    // from a bigger to smaller shake, clear up the borders
    clear_letterbox_borders();
  }

  play.shakesc_amount = amount;
  play.shakesc_delay = delay;
  play.shakesc_length = length;
}


/* *** SCRIPT SYMBOL: [Screen] FadeIn *** */
void FadeIn(int sppd) {
  EndSkippingUntilCharStops();

  if (play.fast_forward)
    return;

  my_fade_in(palette,sppd);
}



void register_screen_script_functions() {
  scAdd_External_Symbol("TintScreen",(void *)TintScreen);
  scAdd_External_Symbol("FadeIn",(void *)FadeIn);
  scAdd_External_Symbol("FadeOut",(void *)my_fade_out);
  scAdd_External_Symbol("FlipScreen",(void *)FlipScreen);
  scAdd_External_Symbol("SetNextScreenTransition",(void *)SetNextScreenTransition);
  scAdd_External_Symbol("SetScreenTransition",(void *)SetScreenTransition);
  scAdd_External_Symbol("ShakeScreen",(void *)ShakeScreen);
  scAdd_External_Symbol("ShakeScreenBackground",(void *)ShakeScreenBackground);
  scAdd_External_Symbol("SetFadeColor",(void *)SetFadeColor);
}

