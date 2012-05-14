using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Text;
using AGS.Types;

namespace AGS.Native
{
    public static class NativeMethodsUtil
    {
        // 2560: start  

#if false
                
        public ref class TempDataStorage
        {
        public:
	        static Room RoomBeingSaved;
        };

        void ConvertStringToCharArray(System.String clrString, char *textBuffer);
        void ConvertStringToCharArray(System.String clrString, char *textBuffer, int maxLength);

        void ThrowManagedException(const char *message) 
        {
	        throw new AGS.Types.AGSEditorException(new String((const char*)message));
        }

        void save_game(bool compressSprites)
        {
	        const char *errorMsg = save_sprites(compressSprites);
	        if (errorMsg != NULL)
	        {
		        throw new AGSEditorException(new String(errorMsg));
	        }
        }

        void CreateBuffer(int width, int height)
        {
	        drawBuffer = create_bitmap_ex(32, width, height);
	        clear_to_color(drawBuffer, 0x00D0D0D0);
        }

        void DrawSpriteToBuffer(int sprNum, int x, int y, int scaleFactor) {
	        block todraw = spriteset[sprNum];
	        if (todraw == NULL)
	          todraw = spriteset[0];

	        if (((thisgame.spriteflags[sprNum] & SPF_640x400) == 0) &&
		        (thisgame.default_resolution > 2))
	        {
		        scaleFactor *= 2;
	        }

	        block imageToDraw = todraw;

	        if (bitmap_color_depth(todraw) != bitmap_color_depth(drawBuffer)) 
	        {
		        int oldColorConv = get_color_conversion();
		        set_color_conversion(oldColorConv | COLORCONV_KEEP_TRANS);
		        block depthConverted = create_bitmap_ex(bitmap_color_depth(drawBuffer), todraw.w, todraw.h);
		        blit(todraw, depthConverted, 0, 0, 0, 0, todraw.w, todraw.h);
		        set_color_conversion(oldColorConv);

		        imageToDraw = depthConverted;
	        }

	        int drawWidth = imageToDraw.w * scaleFactor;
	        int drawHeight = imageToDraw.h * scaleFactor;

	        if ((thisgame.spriteflags[sprNum] & SPF_ALPHACHANNEL) != 0)
	        {
		        if (scaleFactor > 1)
		        {
			        block resizedImage = create_bitmap_ex(bitmap_color_depth(imageToDraw), drawWidth, drawHeight);
			        stretch_blit(imageToDraw, resizedImage, 0, 0, imageToDraw.w, imageToDraw.h, 0, 0, resizedImage.w, resizedImage.h);
			        if (imageToDraw != todraw)
				        destroy_bitmap(imageToDraw);
			        imageToDraw = resizedImage;
		        }
		        set_alpha_blender();
		        draw_trans_sprite(drawBuffer, imageToDraw, x, y);
	        }
	        else
	        {
		        Cstretch_sprite(drawBuffer, imageToDraw, x, y, drawWidth, drawHeight);
	        }

	        if (imageToDraw != todraw)
		        destroy_bitmap(imageToDraw);
        }

        void RenderBufferToHDC(int hdc) 
        {
	        blit_to_hdc(drawBuffer, (HDC)hdc, 0, 0, 0, 0, drawBuffer.w, drawBuffer.h);
	        destroy_bitmap(drawBuffer);
	        drawBuffer = NULL;
        }

#endif

        // 2654
        public static void UpdateSpriteFlags(SpriteFolder folder) 
        {
            foreach (Sprite sprite in folder.Sprites)
            {
                int f = 0;
	            if (sprite.Resolution == Types.SpriteImportResolution.HighRes)
                    f |= acswig.SPF_640x400;
	            if (sprite.AlphaChannel)
                    f |= acswig.SPF_ALPHACHANNEL;

                acswig.thisgame.SetSpriteFlag(sprite.Number, (byte)f);
            }

            foreach (SpriteFolder subFolder in folder.SubFolders) 
            {
	            UpdateSpriteFlags(subFolder);
            }
        }

        // 2671
        public static void GameUpdated(Game game)
        {
            acswig.thisgame.color_depth = (int)game.Settings.ColorDepth;
            acswig.thisgame.default_resolution = (int)game.Settings.Resolution;

            acswig.thisgame.SetOption(acswig.OPT_NOSCALEFNT, game.Settings.FontsForHiRes);
            acswig.thisgame.SetOption(acswig.OPT_ANTIALIASFONTS, game.Settings.AntiAliasFonts);
            acswig.antiAliasFonts = acswig.thisgame.GetOption(acswig.OPT_ANTIALIASFONTS) ? 1 : 0;
            acswig.update_font_sizes();

            acswig.destroy_bitmap(acswig.abuf);
            acswig.abuf = acswig.create_bitmap_ex(acswig.thisgame.color_depth * 8, 32, 32);

            // ensure that the sprite import knows about pal slots 
            for (int i = 0; i < 256; i++)
            {
                if (game.Palette[i].ColourType == Types.PaletteColourType.Background)
                {
                    acswig.thisgame.SetPalUse(i, (byte)acswig.PAL_BACKGROUND);
                }
                else
                {
                    acswig.thisgame.SetPalUse(i, (byte)acswig.PAL_GAMEWIDE);
                }
            }

            acswig.thisgame.numfonts = game.Fonts.Count;
            for (int i = 0; i < acswig.thisgame.numfonts; i++)
            {
                int f = acswig.thisgame.GetFontFlag(i);
                f &= ~acswig.FFLG_SIZEMASK;
                f |= game.Fonts[i].PointSize;
                acswig.thisgame.SetFontFlag(i, (byte)f);
                acswig.reload_font(i);
            }
        }
#if false
                
        void drawViewLoop (int hdc, ViewLoop loopToDraw, int x, int y, int size, int cursel)
        {
          .ViewFrame * frames = (.ViewFrame*)malloc(sizeof(.ViewFrame) * loopToDraw.Frames.Count);
	        for (int i = 0; i < loopToDraw.Frames.Count; i++) 
	        {
		        frames[i].pic = loopToDraw.Frames[i].Image;
		        frames[i].flags = (loopToDraw.Frames[i].Flipped) ? VFLG_FLIPSPRITE : 0;
	        }
          // stretch_sprite is dodgy, retry a few times if it crashes
          int retries = 0;
          while (retries < 3)
          {
            try
            {
	            doDrawViewLoop(hdc, loopToDraw.Frames.Count, frames, x, y, size, cursel);
              break;
            }
            catch (AccessViolationException )
            {
              retries++;
            }
          }
          free(frames);
        }

#endif

        public static unsafe void memcpy(byte* dest, byte* src, int count)
        {
            while (count > 0)
            {
                *dest = *src;
                dest += 1;
                src += 1;

                count -= 1;
            }
        }

        public static unsafe BITMAP CreateBlockFromBitmap(Bitmap bmp, RgbArray imgpal, bool fixColourDepth, bool keepTransparency, out int originalColDepth) 
        {
	        int colDepth;
	        if (bmp.PixelFormat == PixelFormat.Format8bppIndexed)
	        {
		        colDepth = 8;
	        }
	        else if (bmp.PixelFormat == PixelFormat.Format16bppRgb555)
	        {
		        colDepth = 15;
	        }
	        else if (bmp.PixelFormat == PixelFormat.Format16bppRgb565)
	        {
		        colDepth = 16;
	        }
	        else if (bmp.PixelFormat == PixelFormat.Format24bppRgb)
	        {
		        colDepth = 24;
	        }
	        else if (bmp.PixelFormat == PixelFormat.Format32bppRgb)
	        {
		        colDepth = 32;
	        }
	        else if (bmp.PixelFormat == PixelFormat.Format32bppArgb)
	        {
		        colDepth = 32;
	        }
          else if ((bmp.PixelFormat == PixelFormat.Format48bppRgb) ||
                   (bmp.PixelFormat == PixelFormat.Format64bppArgb) ||
                   (bmp.PixelFormat == PixelFormat.Format64bppPArgb))
          {
            throw new AGSEditorException("The source image is 48-bit or 64-bit colour. AGS does not support images with a colour depth higher than 32-bit. Make sure that your paint program is set to produce 32-bit images (8-bit per channel), not 48-bit images (16-bit per channel).");
          }
	        else
	        {
		        throw new AGSEditorException("Unknown pixel format");
	        }

          if ((acswig.thisgame.color_depth == 1) && (colDepth > 8))
          {
            throw new AGSEditorException("You cannot import a hi-colour or true-colour image into a 256-colour game.");
          }

          //if (originalColDepth != NULL)
            originalColDepth = colDepth;

          bool needToFixColourDepth = false;
          if ((colDepth != acswig.thisgame.color_depth * 8) && (fixColourDepth))
          {
            needToFixColourDepth = true;
          }

	        var tempsprite = acswig.create_bitmap_ex(colDepth, bmp.Width, bmp.Height);
	        var rect = new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height);
	        BitmapData bmpData = bmp.LockBits(rect, ImageLockMode.ReadWrite, bmp.PixelFormat);

            byte* address = (byte*)bmpData.Scan0.ToPointer();

	        //unsigned char *address = (unsigned char*)bmpData.Scan0.ToPointer();
	        for (int y = 0; y < tempsprite.h; y++)
	        {
                memcpy((byte*)tempsprite.GetLine(y).ToPointer(), address, bmp.Width * ((colDepth + 1) / 8));
	          address += bmpData.Stride;
	        }
	        bmp.UnlockBits(bmpData);

	        if (colDepth == 8)
	        {
		        System.Drawing.Color[] bmpPalette = bmp.Palette.Entries;
                
		        for (int i = 0; i < 256; i++) {
                    var first = imgpal.getitem(0);
                    var entry = imgpal.getitem(i);
              if (i >= bmpPalette.Length)
              {
                  
                // BMP files can have an arbitrary palette size, fill any
                // missing colours with black
			          entry.r = 1;
			          entry.g = 1;
			          entry.b = 1;
                  
              }
              else
              {
			          entry.r = (byte)(bmpPalette[i].R / 4);
			          entry.g = (byte)(bmpPalette[i].G / 4);
			          entry.b = (byte)(bmpPalette[i].B / 4);

                if ((needToFixColourDepth) && (i > 0) && 
                    (entry.r == first.r) &&
                    (entry.g == first.g) && 
                    (entry.b == first.b))
                {
                  // convert any (0,0,0) colours to (1,1,1) since the image
                  // is about to be converted to hi-colour; this will preserve
                  // any transparency
                  entry.r = (byte)((first.r < 32) ? (first.r + 1) : (first.r - 1));
			            entry.g = (byte)((first.g < 32) ? (first.g + 1) : (first.g - 1));
			            entry.b = (byte)((first.b < 32) ? (first.b + 1) : (first.b - 1));
                }
              }
                    imgpal.setitem(i, entry);
		        }
	        }

	        if (needToFixColourDepth)
	        {
		        var spriteAtRightDepth = acswig.create_bitmap_ex(acswig.thisgame.color_depth * 8, tempsprite.w, tempsprite.h);
		        if (colDepth == 8)
		        {
			        acswig.select_palette(imgpal.cast());
		        }

		        int oldColorConv = acswig.get_color_conversion();
		        if (keepTransparency)
		        {
			        acswig.set_color_conversion(oldColorConv | acswig.COLORCONV_KEEP_TRANS);
		        }
		        else
		        {
			        acswig.set_color_conversion(oldColorConv & ~acswig.COLORCONV_KEEP_TRANS);
		        }

		        acswig.blit(tempsprite, spriteAtRightDepth, 0, 0, 0, 0, tempsprite.w, tempsprite.h);

		        acswig.set_color_conversion(oldColorConv);

		        if (colDepth == 8) 
		        {
			        acswig.unselect_palette();
		        }
		        acswig.destroy_bitmap(tempsprite);
		        tempsprite = spriteAtRightDepth;
	        }

	        if (colDepth > 8) 
	        {
		        acswig.fix_block(tempsprite);
	        }

	        return tempsprite;
        }

#if false
        void DeleteBackground(Room room, int backgroundNumber) 
        {
	        roomstruct *theRoom = (roomstruct*)(void*)room._roomStructPtr;
	        if (theRoom.ebscene[backgroundNumber] != NULL) 
	        {
		        destroy_bitmap(theRoom.ebscene[backgroundNumber]);
		        theRoom.ebscene[backgroundNumber] = NULL;
	        }
	        theRoom.num_bscenes--;
	        room.BackgroundCount--;
	        for (int i = backgroundNumber; i < theRoom.num_bscenes; i++) 
	        {
		        theRoom.ebscene[i] = theRoom.ebscene[i + 1];
		        theRoom.ebpalShared[i] = theRoom.ebpalShared[i + 1];
	        }
        }

        void ImportBackground(Room room, int backgroundNumber, Bitmap bmp, bool useExactPalette, bool sharePalette) 
        {
	        var oldpale = new RgbArray(256);
	        block newbg = CreateBlockFromBitmap(bmp, oldpale, true, false, NULL);
	        roomstruct *theRoom = (roomstruct*)(void*)room._roomStructPtr;
	        theRoom.width = room.Width;
	        theRoom.height = room.Height;
	        bool resolutionChanged = (theRoom.resolution != (int)room.Resolution);
	        theRoom.resolution = (int)room.Resolution;

	        if (bitmap_color_depth(newbg) == 8) 
	        {
		        for (int aa = 0; aa < 256; aa++) {
		          // make sure it maps to locked cols properly
		          if (thisgame.paluses[aa] == PAL_LOCKED)
			          theRoom.bpalettes[backgroundNumber][aa] = palette[aa];
		        }

		        // sharing palette with main background - so copy it across
		        if (sharePalette) {
		          memcpy (&theRoom.bpalettes[backgroundNumber][0], &palette[0], sizeof(color) * 256);
		          theRoom.ebpalShared[backgroundNumber] = 1;
		          if (backgroundNumber >= theRoom.num_bscenes - 1)
		  	        theRoom.ebpalShared[0] = 1;

		          if (!useExactPalette)
			        wremapall(oldpale, newbg, palette);
		        }
		        else {
		          theRoom.ebpalShared[backgroundNumber] = 0;
		          remap_background (newbg, oldpale, theRoom.bpalettes[backgroundNumber], useExactPalette);
		        }

            copy_room_palette_to_global_palette();
	        }

	        if (backgroundNumber >= theRoom.num_bscenes) 
	        {
		        theRoom.num_bscenes++;
	        }
	        else 
	        {
		        destroy_bitmap(theRoom.ebscene[backgroundNumber]);
	        }
	        theRoom.ebscene[backgroundNumber] = newbg;

          // if size or resolution has changed, reset masks
	        if ((newbg.w != theRoom.object.w) || (newbg.h != theRoom.object.h) ||
              (theRoom.width != theRoom.object.w) || (resolutionChanged))
	        {
		        wfreeblock(theRoom.walls);
		        wfreeblock(theRoom.lookat);
		        wfreeblock(theRoom.object);
		        wfreeblock(theRoom.regions);
		        theRoom.walls = create_bitmap_ex(8,theRoom.width / theRoom.resolution, theRoom.height / theRoom.resolution);
		        theRoom.lookat = create_bitmap_ex(8,theRoom.width / theRoom.resolution, theRoom.height / theRoom.resolution);
		        theRoom.object = create_bitmap_ex(8,theRoom.width, theRoom.height);
		        theRoom.regions = create_bitmap_ex(8,theRoom.width / theRoom.resolution, theRoom.height / theRoom.resolution);
		        clear(theRoom.walls);
		        clear(theRoom.lookat);
		        clear(theRoom.object);
		        clear(theRoom.regions);
	        }

	        room.BackgroundCount = theRoom.num_bscenes;
	        room.ColorDepth = bitmap_color_depth(theRoom.ebscene[0]);
        }
#endif

        //2949        
        public static void import_area_mask(IntPtr roomptr, int maskType, Bitmap bmp)
        {
            var oldpale = new RgbArray(256);

            int dummy;
	        var importedImage = CreateBlockFromBitmap(bmp, oldpale, false, false, out dummy);
	        var mask = acswig.get_bitmap_for_mask_from_void(roomptr, (RoomAreaMask)maskType);

	        if (mask.w != importedImage.w)
	        {
		        // allow them to import a double-size or half-size mask, adjust it as appropriate
                acswig.Cstretch_blit(importedImage, mask, 0, 0, importedImage.w, importedImage.h, 0, 0, mask.w, mask.h);
	        }
	        else
	        {
                acswig.blit(importedImage, mask, 0, 0, 0, 0, importedImage.w, importedImage.h);
	        }
            acswig.destroy_bitmap(importedImage);

            acswig.validate_mask(mask, "imported", (maskType == (int)RoomAreaMask.Hotspots) ? acswig.MAX_HOTSPOTS : (acswig.MAX_WALK_AREAS + 1));
        }

#if false
        void set_rgb_mask_from_alpha_channel(block image)
        {
	        for (int y = 0; y < image.h; y++)
	        {
		        unsigned long* thisLine = (unsigned long*)image.line[y];
		        for (int x = 0; x < image.w; x++)
		        {
			        if ((thisLine[x] & 0xff000000) == 0)
			        {
				        thisLine[x] = MASK_COLOR_32;
			        }
		        }
	        }
        }

        void set_opaque_alpha_channel(block image)
        {
	        for (int y = 0; y < image.h; y++)
	        {
		        unsigned long* thisLine = (unsigned long*)image.line[y];
		        for (int x = 0; x < image.w; x++)
		        {
			        if (thisLine[x] != MASK_COLOR_32)
			          thisLine[x] |= 0xff000000;
		        }
	        }
        }
#endif


        //2997
        public static int SetNewSpriteFromBitmap(int slot, Bitmap bmp, int spriteImportMethod, bool remapColours, bool useRoomBackgroundColours, bool alphaChannel) 
        {
            var imgPalBuf = new RgbArray(256);

          int importedColourDepth;
	        var tempsprite = CreateBlockFromBitmap(bmp, imgPalBuf, true, (spriteImportMethod != acswig.SIMP_NONE), out importedColourDepth);

            if ((remapColours) || (acswig.thisgame.color_depth > 1)) 
	        {
                acswig.sort_out_transparency(tempsprite, spriteImportMethod, imgPalBuf.cast(), useRoomBackgroundColours, importedColourDepth);
	        }

            
            acswig.thisgame.SetSpriteFlag(slot, 0);
            if (acswig.thisgame.default_resolution > 2)
	        {
                acswig.thisgame.SetSpriteFlag(slot, (byte)(acswig.thisgame.GetSpriteFlag(slot) | acswig.SPF_640x400));
	        }
	        if (alphaChannel)
	        {
                acswig.thisgame.SetSpriteFlag(slot, (byte)(acswig.thisgame.GetSpriteFlag(slot) | acswig.SPF_ALPHACHANNEL));

                if (acswig.bitmap_color_depth(tempsprite) == 32)
		        {
                    acswig.set_rgb_mask_from_alpha_channel(tempsprite);
		        }
	        }
            else if (acswig.bitmap_color_depth(tempsprite) == 32)
	        {
                acswig.set_opaque_alpha_channel(tempsprite);
	        }

            acswig.SetNewSprite(slot, tempsprite);

            return (acswig.thisgame.default_resolution > 2) ? 1 : 0;
        }

        // 3032
        public static void SetBitmapPaletteFromGlobalPalette(System.Drawing.Bitmap bmp)
        {
	        ColorPalette colorPal = bmp.Palette;
            Color[] bmpPalette = colorPal.Entries;
            var palette = RgbArray.frompointer(acswig.palette);
	        for (int i = 0; i < 256; i++)
	        {
	            RGB entry = palette.getitem(i);
                bmpPalette[i] = Color.FromArgb((i == 0) ? i : 255, entry.r * 4, entry.g * 4, entry.b * 4);
	        }

	        // Need to set this back to make it pick up the changes
	        bmp.Palette = colorPal;
	        //bmp.MakeTransparent(bmpPalette[0]);
        }

        // 3046
        public static unsafe Bitmap ConvertBlockToBitmap(BITMAP todraw, bool useAlphaChannel) 
        {
          acswig.fix_block(todraw);

          PixelFormat pixFormat = PixelFormat.Format32bppRgb;
          if ((acswig.bitmap_color_depth(todraw) == 32) && (useAlphaChannel))
	          pixFormat = PixelFormat.Format32bppArgb;
          else if (acswig.bitmap_color_depth(todraw) == 24)
            pixFormat = PixelFormat.Format24bppRgb;
          else if (acswig.bitmap_color_depth(todraw) == 16)
            pixFormat = PixelFormat.Format16bppRgb565;
          else if (acswig.bitmap_color_depth(todraw) == 15)
            pixFormat = PixelFormat.Format16bppRgb555;
          else if (acswig.bitmap_color_depth(todraw) == 8)
            pixFormat = PixelFormat.Format8bppIndexed;
          int bytesPerPixel = (acswig.bitmap_color_depth(todraw) + 1) / 8;

          Bitmap bmp = new System.Drawing.Bitmap(todraw.w, todraw.h, pixFormat);
          var rect = new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height);
          BitmapData bmpData = bmp.LockBits(rect, ImageLockMode.WriteOnly, bmp.PixelFormat);
          byte *address = (byte*)bmpData.Scan0.ToPointer();
          for (int y = 0; y < todraw.h; y++) {
            memcpy(address, (byte*)todraw.GetLine(y).ToPointer(), bmp.Width * bytesPerPixel);
            address += bmpData.Stride;
          }
          bmp.UnlockBits(bmpData);

          if (pixFormat == PixelFormat.Format8bppIndexed)
            SetBitmapPaletteFromGlobalPalette(bmp);

          acswig.fix_block(todraw);
          return bmp;
        }

        // 3080
        public static unsafe System.Drawing.Bitmap ConvertBlockToBitmap32(BITMAP todraw, int width, int height, bool useAlphaChannel) 
        {
          var tempBlock = acswig.create_bitmap_ex(32, todraw.w, todraw.h);
        	
          if (acswig.bitmap_color_depth(todraw) == 8)
              acswig.select_palette(acswig.palette);

          acswig.blit(todraw, tempBlock, 0, 0, 0, 0, todraw.w, todraw.h);

          if (acswig.bitmap_color_depth(todraw) == 8)
	        acswig.unselect_palette();

          if ((width != todraw.w) || (height != todraw.h)) 
          {
	          var newBlock = acswig.create_bitmap_ex(32, width, height);
	          acswig.Cstretch_blit(tempBlock, newBlock, 0, 0, todraw.w, todraw.h, 0, 0, width, height);
	          acswig.destroy_bitmap(tempBlock);
	          tempBlock = newBlock;
          }

          acswig.fix_block(tempBlock);

          PixelFormat pixFormat = PixelFormat.Format32bppRgb;
          if ((acswig.bitmap_color_depth(todraw) == 32) && (useAlphaChannel))
	          pixFormat = PixelFormat.Format32bppArgb;

          Bitmap bmp = new System.Drawing.Bitmap(width, height, pixFormat);
          var rect = new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height);
          BitmapData bmpData = bmp.LockBits(rect, ImageLockMode.WriteOnly, bmp.PixelFormat);
          byte *address = (byte*)bmpData.Scan0.ToPointer();
          for (int y = 0; y < tempBlock.h; y++) {
            memcpy(address, (byte*)tempBlock.GetLine(y).ToPointer(), bmp.Width * 4);
            address += bmpData.Stride;
          }
          bmp.UnlockBits(bmpData);
          acswig.destroy_bitmap(tempBlock);
          return bmp;
        }

        // 3119
        public static unsafe System.Drawing.Bitmap ConvertAreaMaskToBitmap(BITMAP mask) 
        {
	        Bitmap bmp = new System.Drawing.Bitmap(mask.w, mask.h, PixelFormat.Format8bppIndexed);
	        var rect = new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height);
	        BitmapData bmpData = bmp.LockBits(rect, ImageLockMode.WriteOnly, bmp.PixelFormat);
	        byte *address = (byte*)bmpData.Scan0.ToPointer();
	        for (int y = 0; y < mask.h; y++) 
	        {
                memcpy(address, (byte*)mask.GetLine(y).ToPointer(), bmp.Width);
		        address += bmpData.Stride;
	        }
	        bmp.UnlockBits(bmpData);

          SetBitmapPaletteFromGlobalPalette(bmp);

	        return bmp;
        }

        // 3137
        public static System.Drawing.Bitmap getSpriteAsBitmap(int spriteNum) {
            BITMAP todraw = acswig.get_sprite(spriteNum);
            return ConvertBlockToBitmap(todraw, (acswig.thisgame.GetSpriteFlag(spriteNum) & acswig.SPF_ALPHACHANNEL) != 0);
        }

        // 3142
        public static System.Drawing.Bitmap getSpriteAsBitmap32bit(int spriteNum, int width, int height) {
            BITMAP todraw = acswig.get_sprite(spriteNum);
          if (todraw == null)
          {
	          throw new AGSEditorException(String.Format("getSpriteAsBitmap32bit: Unable to find sprite {0}", spriteNum));
          }
          return ConvertBlockToBitmap32(todraw, width, height, (acswig.thisgame.GetSpriteFlag(spriteNum) & acswig.SPF_ALPHACHANNEL) != 0);
        }
    
        // 3151
        public static System.Drawing.Bitmap getBackgroundAsBitmap(Room room, int backgroundNumber) {
            var roomptr = acswig.cast_to_roomstruct(room._roomStructPtr);
          return ConvertBlockToBitmap32(roomptr.GetEbScene(backgroundNumber), room.Width, room.Height, false);
        }

        // 3157
        public static void PaletteUpdated(PaletteEntry[] newPalette) 
        {  
	        foreach (PaletteEntry colour in newPalette)
	        {
	            var c = acswig.get_palette_entry(colour.Index);
                c.r = (byte)(colour.Colour.R / 4);
                c.g = (byte)(colour.Colour.G / 4);
                c.b = (byte)(colour.Colour.B / 4);
	            acswig.set_palette_entry(colour.Index, c);
	        }
            acswig.allegro_set_palette(acswig.palette);
            acswig.copy_global_palette_to_room_palette();
        }

#if false
        void ConvertGUIToBinaryFormat(GUI guiObj, GUIMain *gui) 
        {
          NormalGUI normalGui = dynamic_cast<NormalGUI>(guiObj);
          if (normalGui)
          {
	        ConvertStringToCharArray(normalGui.OnClick, gui.clickEventHandler, 20);
	        gui.x = normalGui.Left;
	        gui.y = normalGui.Top;
	        gui.wid = normalGui.Width;
	        gui.hit = normalGui.Height;
	        gui.flags = (normalGui.Clickable) ? 0 : GUIF_NOCLICK;
            gui.popupyp = normalGui.PopupYPos;
            gui.popup = (int)normalGui.Visibility;
            gui.zorder = normalGui.ZOrder;
	        gui.vtext[0] = 0;
            gui.fgcol = normalGui.BorderColor;
            gui.SetTransparencyAsPercentage(normalGui.Transparency);
          }
          else
          {
            TextWindowGUI twGui = dynamic_cast<TextWindowGUI>(guiObj);
	        gui.wid = 200;
	        gui.hit = 100;
            gui.flags = 0;
	        gui.popup = POPUP_SCRIPT;
	        gui.vtext[0] = GUI_TEXTWINDOW;
          gui.fgcol = twGui.TextColor;
          }
          gui.bgcol = guiObj.BackgroundColor;
          gui.bgpic = guiObj.BackgroundImage;
          
          ConvertStringToCharArray(guiObj.Name, gui.name);

          gui.numobjs = 0;

          for each (GUIControl control in guiObj.Controls)
          {
	          AGS.Types.GUIButton button = dynamic_cast<AGS.Types.GUIButton>(control);
	          AGS.Types.GUILabel label = dynamic_cast<AGS.Types.GUILabel>(control);
	          AGS.Types.GUITextBox textbox = dynamic_cast<AGS.Types.GUITextBox>(control);
	          AGS.Types.GUIListBox listbox = dynamic_cast<AGS.Types.GUIListBox>(control);
	          AGS.Types.GUISlider slider = dynamic_cast<AGS.Types.GUISlider>(control);
	          AGS.Types.GUIInventory invwindow = dynamic_cast<AGS.Types.GUIInventory>(control);
	          AGS.Types.GUITextWindowEdge textwindowedge = dynamic_cast<AGS.Types.GUITextWindowEdge>(control);
	          if (button)
	          {
		          guibuts[numguibuts].textcol = button.TextColor;
		          guibuts[numguibuts].font = button.Font;
		          guibuts[numguibuts].pic = button.Image;
		          guibuts[numguibuts].usepic = guibuts[numguibuts].pic;
		          guibuts[numguibuts].overpic = button.MouseoverImage;
		          guibuts[numguibuts].pushedpic = button.PushedImage;
		          guibuts[numguibuts].textAlignment = (int)button.TextAlignment;
		          guibuts[numguibuts].leftclick = (int)button.ClickAction;
		          guibuts[numguibuts].lclickdata = button.NewModeNumber;
		          guibuts[numguibuts].flags = (button.ClipImage) ? GUIF_CLIP : 0;
		          ConvertStringToCharArray(button.Text, guibuts[numguibuts].text, 50);
		          ConvertStringToCharArray(button.OnClick, guibuts[numguibuts].eventHandlers[0], MAX_GUIOBJ_EVENTHANDLER_LEN + 1);
        		  
		          gui.objrefptr[gui.numobjs] = (GOBJ_BUTTON << 16) | numguibuts;
		          gui.objs[gui.numobjs] = &guibuts[numguibuts];
		          gui.numobjs++;
		          numguibuts++;
	          }
	          else if (label)
	          {
		          guilabels[numguilabels].textcol = label.TextColor;
		          guilabels[numguilabels].font = label.Font;
		          guilabels[numguilabels].align = (int)label.TextAlignment;
		          guilabels[numguilabels].flags = 0;
		          char textBuffer[MAX_GUILABEL_TEXT_LEN];
		          ConvertStringToCharArray(label.Text, textBuffer, MAX_GUILABEL_TEXT_LEN);
		          guilabels[numguilabels].SetText(textBuffer);

		          gui.objrefptr[gui.numobjs] = (GOBJ_LABEL << 16) | numguilabels;
		          gui.objs[gui.numobjs] = &guilabels[numguilabels];
		          gui.numobjs++;
		          numguilabels++;
	          }
	          else if (textbox)
	          {
		          guitext[numguitext].textcol = textbox.TextColor;
		          guitext[numguitext].font = textbox.Font;
		          guitext[numguitext].flags = 0;
		          guitext[numguitext].exflags = (textbox.ShowBorder) ? 0 : GTF_NOBORDER;
		          guitext[numguitext].text[0] = 0;
		          ConvertStringToCharArray(textbox.OnActivate, guitext[numguitext].eventHandlers[0], MAX_GUIOBJ_EVENTHANDLER_LEN + 1);

		          gui.objrefptr[gui.numobjs] = (GOBJ_TEXTBOX << 16) | numguitext;
		          gui.objs[gui.numobjs] = &guitext[numguitext];
		          gui.numobjs++;
		          numguitext++;
	          }
	          else if (listbox)
	          {
		          guilist[numguilist].textcol = listbox.TextColor;
		          guilist[numguilist].font = listbox.Font;
		          guilist[numguilist].backcol = listbox.SelectedTextColor;
		          guilist[numguilist].selectedbgcol = listbox.SelectedBackgroundColor;
		          guilist[numguilist].alignment = (int)listbox.TextAlignment;
		          guilist[numguilist].exflags = (listbox.ShowBorder) ? 0 : GLF_NOBORDER;
		          guilist[numguilist].exflags |= (listbox.ShowScrollArrows) ? 0 : GLF_NOARROWS;
		          ConvertStringToCharArray(listbox.OnSelectionChanged, guilist[numguilist].eventHandlers[0], MAX_GUIOBJ_EVENTHANDLER_LEN + 1);

		          gui.objrefptr[gui.numobjs] = (GOBJ_LISTBOX << 16) | numguilist;
		          gui.objs[gui.numobjs] = &guilist[numguilist];
		          gui.numobjs++;
		          numguilist++;
	          }
	          else if (slider)
	          {
		          guislider[numguislider].min = slider.MinValue;
		          guislider[numguislider].max = slider.MaxValue;
		          guislider[numguislider].value = slider.Value;
		          guislider[numguislider].handlepic = slider.HandleImage;
		          guislider[numguislider].handleoffset = slider.HandleOffset;
		          guislider[numguislider].bgimage = slider.BackgroundImage;
		          ConvertStringToCharArray(slider.OnChange, guislider[numguislider].eventHandlers[0], MAX_GUIOBJ_EVENTHANDLER_LEN + 1);

		          gui.objrefptr[gui.numobjs] = (GOBJ_SLIDER << 16) | numguislider;
		          gui.objs[gui.numobjs] = &guislider[numguislider];
		          gui.numobjs++;
		          numguislider++;
	          }
	          else if (invwindow)
	          {
		          guiinv[numguiinv].charId = invwindow.CharacterID;
		          guiinv[numguiinv].itemWidth = invwindow.ItemWidth;
		          guiinv[numguiinv].itemHeight = invwindow.ItemHeight;

		          gui.objrefptr[gui.numobjs] = (GOBJ_INVENTORY << 16) | numguiinv;
		          gui.objs[gui.numobjs] = &guiinv[numguiinv];
		          gui.numobjs++;
		          numguiinv++;
	          }
	          else if (textwindowedge)
	          {
		          guibuts[numguibuts].pic = textwindowedge.Image;
		          guibuts[numguibuts].usepic = guibuts[numguibuts].pic;
		          guibuts[numguibuts].flags = 0;
		          guibuts[numguibuts].text[0] = 0;
        		  
		          gui.objrefptr[gui.numobjs] = (GOBJ_BUTTON << 16) | numguibuts;
		          gui.objs[gui.numobjs] = &guibuts[numguibuts];
		          gui.numobjs++;
		          numguibuts++;
	          }

	          GUIObject *newObj = gui.objs[gui.numobjs - 1];
	          newObj.x = control.Left;
	          newObj.y = control.Top;
	          newObj.wid = control.Width;
	          newObj.hit = control.Height;
	          newObj.objn = control.ID;
	          newObj.zorder = control.ZOrder;
	          ConvertStringToCharArray(control.Name, newObj.scriptName, MAX_GUIOBJ_SCRIPTNAME_LEN + 1);
          }

          gui.rebuild_array();
          gui.resort_zorder();
        }

        void drawGUI(int hdc, int x,int y, GUI guiObj, int scaleFactor, int selectedControl) {
          numguibuts = 0;
          numguilabels = 0;
          numguitext = 0;
          numguilist = 0;
          numguislider = 0;
          numguiinv = 0;

          ConvertGUIToBinaryFormat(guiObj, &tempgui);

          tempgui.highlightobj = selectedControl;

          drawGUIAt(hdc, x, y, -1, -1, -1, -1, scaleFactor);
        }

        Dictionary<int, Sprite> load_sprite_dimensions()
        {
	        Dictionary<int, Sprite> sprites = new Dictionary<int, Sprite>();

	        for (int i = 0; i < spriteset.elements; i++)
	        {
		        block spr = spriteset[i];
		        if (spr != NULL)
		        {
			        sprites.Add(i, new Sprite(i, spr.w, spr.h, bitmap_color_depth(spr), (thisgame.spriteflags[i] & SPF_640x400) ? SpriteImportResolution.HighRes : SpriteImportResolution.LowRes, (thisgame.spriteflags[i] & SPF_ALPHACHANNEL) ? true : false));
		        }
	        }

	        return sprites;
        }

        void ConvertCustomProperties(AGS.Types.CustomProperties insertInto, .CustomProperties *propToConvert)
        {
	        for (int j = 0; j < propToConvert.numProps; j++) 
	        {
		        CustomProperty newProp = new CustomProperty();
		        newProp.Name = new String(propToConvert.propName[j]);
		        newProp.Value = new String(propToConvert.propVal[j]);
		        insertInto.PropertyValues.Add(newProp.Name, newProp);
	        }
        }

        void CompileCustomProperties(AGS.Types.CustomProperties convertFrom, .CustomProperties *compileInto)
        {
	        compileInto.reset();
	        int j = 0;
	        char propName[200];
	        char propVal[MAX_CUSTOM_PROPERTY_VALUE_LENGTH];
	        for each (String key in convertFrom.PropertyValues.Keys)
	        {
		        ConvertStringToCharArray(convertFrom.PropertyValues[key].Name, propName, 200);
		        ConvertStringToCharArray(convertFrom.PropertyValues[key].Value, propVal, MAX_CUSTOM_PROPERTY_VALUE_LENGTH);
		        compileInto.addProperty(propName, propVal);
		        j++;
	        }
        }

        char charScriptNameBuf[100];
        const char *GetCharacterScriptName(int charid, AGS.Types.Game game) 
        {
	        if ((charid >= 0) && (game != nullptr) &&
            (charid < game.Characters.Count) &&
		        (game.Characters[charid].ScriptName.Length > 0))
	        {
		        ConvertStringToCharArray(game.Characters[charid].ScriptName,  charScriptNameBuf, 100); 
	        }
	        else 
	        {
		        sprintf(charScriptNameBuf, "character[%d]", charid);
	        }
	        return charScriptNameBuf;
        }

        void ConvertInteractionToScript(System.Text.StringBuilder sb, NewInteractionCommand *intrcmd, String scriptFuncPrefix, AGS.Types.Game game, int *runScriptCount, bool *onlyIfInvWasUseds, int commandOffset) 
        {
          if (intrcmd.type != 1)
          {
            // if another type of interaction, we definately can't optimise
            // away the wrapper function
            runScriptCount[0] = 1000;
          }
          else
          {
            runScriptCount[0]++;
          }

          if (intrcmd.type != 20)
          {
	          *onlyIfInvWasUseds = false;
          }

	        switch (intrcmd.type)
	        {
	        case 0:
		        break;
	        case 1:  // Run Script
		        sb.Append(scriptFuncPrefix);
		        sb.Append(System.Convert.ToChar(intrcmd.data[0].val + 'a'));
		        sb.AppendLine("();");
		        break;
	        case 3: // Add Score
	        case 4: // Display Message
	        case 5: // Play Music
	        case 6: // Stop Music
	        case 7: // Play Sound
	        case 8: // Play Flic
	        case 9: // Run Dialog
	        case 10: // Enable Dialog Option
	        case 11: // Disalbe Dialog Option
	        case 13: // Give player an inventory item
	        case 15: // hide object
	        case 16: // show object 
	        case 17: // change object view
	        case 20: // IF inv was used
	        case 21: // IF player has inv
	        case 22: // IF character is moving
	        case 24: // stop moving
	        case 25: // change room (at co-ords)
	        case 26: // change room of NPC
	        case 27: // lock character view
	        case 28: // unlock character view
	        case 29: // follow character
	        case 30: // stop following
	        case 31: // disable hotspot
	        case 32: // enable hotspot
	        case 36: // set idle
	        case 37: // disable idle
	        case 38: // lose inventory
	        case 39: // show gui
	        case 40: // hide gui
	        case 41: // stop running commands
	        case 42: // facelocation
	        case 43: // wait()
	        case 44: // change character view
	        case 45: // IF player is
	        case 46: // IF mouse cursor is
	        case 47: // IF player has been in room
		        // For these, the sample script code will work
		        {
		        String scriptCode = new String(actions[intrcmd.type].textscript);
		        if ((*onlyIfInvWasUseds) && (commandOffset > 0))
		        {
			        scriptCode = String.Concat("else ", scriptCode);
		        }
		        scriptCode = scriptCode.Replace("$$1", (new Int32(intrcmd.data[0].val)).ToString() );
		        scriptCode = scriptCode.Replace("$$2", (new Int32(intrcmd.data[1].val)).ToString() );
		        scriptCode = scriptCode.Replace("$$3", (new Int32(intrcmd.data[2].val)).ToString() );
		        scriptCode = scriptCode.Replace("$$4", (new Int32(intrcmd.data[3].val)).ToString() );
		        sb.AppendLine(scriptCode);
		        }
		        break;
	        case 34: // animate character
		        {
		        char scriptCode[100];
		        int charID = intrcmd.data[0].val;
		        int loop = intrcmd.data[1].val;
		        int speed = intrcmd.data[2].val;
		        sprintf(scriptCode, "%s.Animate(%d, %d, eOnce, eBlock);", GetCharacterScriptName(charID, game), loop, speed);
		        sb.AppendLine(new String(scriptCode));
		        }
		        break;
	        case 35: // quick animation
		        {
		        char scriptCode[300];
		        int charID = intrcmd.data[0].val;
		        int view = intrcmd.data[1].val;
		        int loop = intrcmd.data[2].val;
		        int speed = intrcmd.data[3].val;
		        sprintf(scriptCode, "%s.LockView(%d);\n"
							        "%s.Animate(%d, %d, eOnce, eBlock);\n"
							        "%s.UnlockView();",
							        GetCharacterScriptName(charID, game), view, 
							        GetCharacterScriptName(charID, game), loop, speed, 
							        GetCharacterScriptName(charID, game));
		        sb.AppendLine(new String(scriptCode));
		        }
		        break;
	        case 14: // Move Object
		        {
		        char scriptCode[100];
		        int objID = intrcmd.data[0].val;
		        int x = intrcmd.data[1].val;
		        int y = intrcmd.data[2].val;
		        int speed = intrcmd.data[3].val;
		        sprintf(scriptCode, "object[%d].Move(%d, %d, %d, %s);", objID, x, y, speed, (intrcmd.data[4].val) ? "eBlock" : "eNoBlock");
		        sb.AppendLine(new String(scriptCode));
		        }
		        break;
	        case 19: // Move Character
		        {
		        char scriptCode[100];
		        int charID = intrcmd.data[0].val;
		        int x = intrcmd.data[1].val;
		        int y = intrcmd.data[2].val;
		        sprintf(scriptCode, "%s.Walk(%d, %d, %s);", GetCharacterScriptName(charID, game), x, y, (intrcmd.data[3].val) ? "eBlock" : "eNoBlock");
		        sb.AppendLine(new String(scriptCode));
		        }
		        break;
	        case 18: // Animate Object
		        {
		        char scriptCode[100];
		        int objID = intrcmd.data[0].val;
		        int loop = intrcmd.data[1].val;
		        int speed = intrcmd.data[2].val;
		        sprintf(scriptCode, "object[%d].Animate(%d, %d, %s, eNoBlock);", objID, loop, speed, (intrcmd.data[3].val) ? "eRepeat" : "eOnce");
		        sb.AppendLine(new String(scriptCode));
		        }
		        break;
	        case 23: // IF variable set to value
		        {
		        char scriptCode[100];
		        int valueToCheck = intrcmd.data[1].val;
		        if ((game == nullptr) || (intrcmd.data[0].val >= game.OldInteractionVariables.Count))
		        {
			        sprintf(scriptCode, "if (__INTRVAL$%d$ == %d) {", intrcmd.data[0].val, valueToCheck);
		        }
		        else
		        {
			        OldInteractionVariable variableToCheck = game.OldInteractionVariables[intrcmd.data[0].val];
			        sprintf(scriptCode, "if (%s == %d) {", variableToCheck.ScriptName, valueToCheck);
		        }
		        sb.AppendLine(new String(scriptCode));
		        break;
		        }
	        case 33: // Set variable
		        {
		        char scriptCode[100];
		        int valueToCheck = intrcmd.data[1].val;
		        if ((game == nullptr) || (intrcmd.data[0].val >= game.OldInteractionVariables.Count))
		        {
			        sprintf(scriptCode, "__INTRVAL$%d$ = %d;", intrcmd.data[0].val, valueToCheck);
		        }
		        else
		        {
			        OldInteractionVariable variableToCheck = game.OldInteractionVariables[intrcmd.data[0].val];
			        sprintf(scriptCode, "%s = %d;", variableToCheck.ScriptName, valueToCheck);
		        }
		        sb.AppendLine(new String(scriptCode));
		        break;
		        }
	        case 12: // Change Room
		        {
		        char scriptCode[200];
		        int room = intrcmd.data[0].val;
		        sprintf(scriptCode, "player.ChangeRoomAutoPosition(%d", room);
		        if (intrcmd.data[1].val > 0) 
		        {
			        sprintf(&scriptCode[strlen(scriptCode)], ", %d", intrcmd.data[1].val);
		        }
		        strcat(scriptCode, ");");
		        sb.AppendLine(new String(scriptCode));
		        }
		        break;
	        case 2: // Add Score On First Execution
		        {
		          int points = intrcmd.data[0].val;
              String newGuid = System.Guid.NewGuid().ToString();
              String scriptCode = String.Format("if (Game.DoOnceOnly(\"{0}\"))", newGuid);
              scriptCode = String.Concat(scriptCode, " {\n  ");
              scriptCode = String.Concat(scriptCode, String.Format("GiveScore({0});", points.ToString()));
              scriptCode = String.Concat(scriptCode, "\n}");
		          sb.AppendLine(scriptCode);
		        }
		        break;
	        default:
		        throw new InvalidDataException("Invalid interaction type found");
	        }
        }

        void ConvertInteractionCommandList(System.Text.StringBuilder sb, NewInteractionCommandList *cmdList, String scriptFuncPrefix, AGS.Types.Game game, int *runScriptCount, int targetTypeForUnhandledEvent) 
        {
	        bool onlyIfInvWasUseds = true;

	        for (int cmd = 0; cmd < cmdList.numCommands; cmd++)
	        {
		        ConvertInteractionToScript(sb, &cmdList.command[cmd], scriptFuncPrefix, game, runScriptCount, &onlyIfInvWasUseds, cmd);
		        if (cmdList.command[cmd].get_child_list() != NULL) 
		        {
			        ConvertInteractionCommandList(sb, cmdList.command[cmd].get_child_list(), scriptFuncPrefix, game, runScriptCount, targetTypeForUnhandledEvent);
			        sb.AppendLine("}");
		        }
	        }

	        if ((onlyIfInvWasUseds) && (targetTypeForUnhandledEvent > 0) && 
		        (cmdList.numCommands > 0))
	        {
		        sb.AppendLine("else {");
		        sb.AppendLine(String.Format(" unhandled_event({0}, 3);", targetTypeForUnhandledEvent));
		        sb.AppendLine("}");
	        }
        }

        void CopyInteractions(AGS.Types.Interactions destination, .InteractionScripts *source)
        {
	        if (source.numEvents > destination.ScriptFunctionNames.Length) 
	        {
		        throw new AGS.Types.AGSEditorException("Invalid interaction funcs: too many interaction events");
	        }

	        for (int i = 0; i < source.numEvents; i++) 
	        {
		        destination.ScriptFunctionNames[i] = new String(source.scriptFuncNames[i]);
	        }
        }

        void ConvertInteractions(AGS.Types.Interactions interactions, .NewInteraction *intr, String scriptFuncPrefix, AGS.Types.Game game, int targetTypeForUnhandledEvent)
        {
	        if (intr.numEvents > interactions.ScriptFunctionNames.Length) 
	        {
		        throw new AGS.Types.AGSEditorException("Invalid interaction data: too many interaction events");
	        }

	        for (int i = 0; i < intr.numEvents; i++) 
	        {
		        if (intr.response[i] != NULL) 
		        {
              int runScriptCount = 0;
			        System.Text.StringBuilder sb = new System.Text.StringBuilder();
			        ConvertInteractionCommandList(sb, intr.response[i], scriptFuncPrefix, game, &runScriptCount, targetTypeForUnhandledEvent);
              if (runScriptCount == 1)
              {
                sb.Append("$$SINGLE_RUN_SCRIPT$$");
              }
			        interactions.ImportedScripts[i] = sb.ToString();
		        }
	        }
        }

        Game load_old_game_dta_file(const char *fileName)
        {
	        const char *errorMsg = load_dta_file_into_thisgame(fileName);
	        if (errorMsg != NULL)
	        {
		        throw new AGS.Types.AGSEditorException(new String(errorMsg));
	        }

	        Game game = new Game();
	        game.Settings.AlwaysDisplayTextAsSpeech = (thisgame.options[OPT_ALWAYSSPCH] != 0);
	        game.Settings.AntiAliasFonts = (thisgame.options[OPT_ANTIALIASFONTS] != 0);
	        game.Settings.AntiGlideMode = (thisgame.options[OPT_ANTIGLIDE] != 0);
	        game.Settings.AutoMoveInWalkMode = !thisgame.options[OPT_NOWALKMODE];
	        game.Settings.BackwardsText = (thisgame.options[OPT_RIGHTLEFTWRITE] != 0);
	        game.Settings.ColorDepth = (GameColorDepth)thisgame.color_depth;
	        game.Settings.CompressSprites = (thisgame.options[OPT_COMPRESSSPRITES] != 0);
	        game.Settings.CrossfadeMusic = (CrossfadeSpeed)thisgame.options[OPT_CROSSFADEMUSIC];
	        game.Settings.DebugMode = (thisgame.options[OPT_DEBUGMODE] != 0);
	        game.Settings.DialogOptionsBackwards = (thisgame.options[OPT_DIALOGUPWARDS] != 0);
	        game.Settings.DialogOptionsGap = thisgame.options[OPT_DIALOGGAP];
	        game.Settings.DialogOptionsGUI = thisgame.options[OPT_DIALOGIFACE];
	        game.Settings.DialogOptionsBullet = thisgame.dialog_bullet;
	        game.Settings.DisplayMultipleInventory = (thisgame.options[OPT_DUPLICATEINV] != 0);
	        game.Settings.EnforceNewStrings = (thisgame.options[OPT_STRICTSTRINGS] != 0);
          game.Settings.EnforceNewAudio = false;
	        game.Settings.EnforceObjectBasedScript = (thisgame.options[OPT_STRICTSCRIPTING] != 0);
	        game.Settings.FontsForHiRes = (thisgame.options[OPT_NOSCALEFNT] != 0);
	        game.Settings.GameName = new String(thisgame.gamename);
	        game.Settings.GUIAlphaStyle = GUIAlphaStyle.Classic;
	        game.Settings.HandleInvClicksInScript = (thisgame.options[OPT_HANDLEINVCLICKS] != 0);
	        game.Settings.InventoryCursors = !thisgame.options[OPT_FIXEDINVCURSOR];
	        game.Settings.LeftToRightPrecedence = (thisgame.options[OPT_LEFTTORIGHTEVAL] != 0);
	        game.Settings.LetterboxMode = (thisgame.options[OPT_LETTERBOX] != 0);
	        game.Settings.MaximumScore = thisgame.totalscore;
	        game.Settings.MouseWheelEnabled = (thisgame.options[OPT_MOUSEWHEEL] != 0);
	        game.Settings.NumberDialogOptions = (thisgame.options[OPT_DIALOGNUMBERED] != 0);
	        game.Settings.PixelPerfect = (thisgame.options[OPT_PIXPERFECT] != 0);
	        game.Settings.PlaySoundOnScore = thisgame.options[OPT_SCORESOUND];
	        game.Settings.Resolution = (GameResolutions)thisgame.default_resolution;
	        game.Settings.RoomTransition = (RoomTransitionStyle)thisgame.options[OPT_FADETYPE];
	        game.Settings.SaveScreenshots = (thisgame.options[OPT_SAVESCREENSHOT] != 0);
	        game.Settings.SkipSpeech = (SkipSpeechStyle)thisgame.options[OPT_NOSKIPTEXT];
	        game.Settings.SpeechPortraitSide = (SpeechPortraitSide)thisgame.options[OPT_PORTRAITSIDE];
	        game.Settings.SpeechStyle = (SpeechStyle)thisgame.options[OPT_SPEECHTYPE];
	        game.Settings.SplitResources = thisgame.options[OPT_SPLITRESOURCES];
	        game.Settings.TextWindowGUI = thisgame.options[OPT_TWCUSTOM];
	        game.Settings.ThoughtGUI = thisgame.options[OPT_THOUGHTGUI];
	        game.Settings.TurnBeforeFacing = (thisgame.options[OPT_TURNTOFACELOC] != 0);
	        game.Settings.TurnBeforeWalking = (thisgame.options[OPT_ROTATECHARS] != 0);
	        game.Settings.WalkInLookMode = (thisgame.options[OPT_WALKONLOOK] != 0);
	        game.Settings.WhenInterfaceDisabled = (InterfaceDisabledAction)thisgame.options[OPT_DISABLEOFF];
	        game.Settings.UniqueID = thisgame.uniqueid;
          game.Settings.SaveGameFolderName = new String(thisgame.gamename);

	        game.Settings.InventoryHotspotMarker.DotColor = thisgame.hotdot;
	        game.Settings.InventoryHotspotMarker.CrosshairColor = thisgame.hotdotouter;
	        game.Settings.InventoryHotspotMarker.Image = thisgame.invhotdotsprite;
	        if (thisgame.invhotdotsprite) 
	        {
		        game.Settings.InventoryHotspotMarker.Style = InventoryHotspotMarkerStyle.Sprite;
	        }
	        else if (thisgame.hotdot) 
	        {
		        game.Settings.InventoryHotspotMarker.Style = InventoryHotspotMarkerStyle.Crosshair;
	        }
	        else 
	        {
		        game.Settings.InventoryHotspotMarker.Style = InventoryHotspotMarkerStyle.None;
	        }

	        int i;
	        for (i = 0; i < 256; i++) 
	        {
		        if (thisgame.paluses[i] == PAL_BACKGROUND) 
		        {
			        game.Palette[i].ColourType = PaletteColourType.Background;
		        }
		        else 
		        {
			        game.Palette[i].ColourType = PaletteColourType.Gamewide; 
		        }
		        game.Palette[i].Colour = Color.FromArgb(palette[i].r * 4, palette[i].g * 4, palette[i].b * 4);
	        }

	        for (i = 0; i < numThisgamePlugins; i++) 
	        {
		        cli.array<System.Byte> pluginData = new cli.array<System.Byte>(thisgamePlugins[i].dataSize);
		        for (int j = 0; j < thisgamePlugins[i].dataSize; j++) 
		        {
			        pluginData[j] = thisgamePlugins[i].data[j];
		        }
        		
		        AGS.Types.Plugin plugin = new AGS.Types.Plugin(new String(thisgamePlugins[i].filename), pluginData);
		        game.Plugins.Add(plugin);
	        }

	        for (i = 0; i < numGlobalVars; i++)
	        {
		        OldInteractionVariable intVar;
		        intVar = new OldInteractionVariable(new String(globalvars[i].name), globalvars[i].value);
		        game.OldInteractionVariables.Add(intVar);
	        }
        	
	        AGS.Types.ViewFolder viewFolder = new AGS.Types.ViewFolder();
	        for (i = 0; i < thisgame.numviews; i++) 
	        {
		        AGS.Types.View view = new AGS.Types.View();
		        view.Name = new String(thisgame.viewNames[i]);
		        view.ID = i + 1;

		        for (int j = 0; j < oldViews[i].numloops; j++) 
		        {
			        ViewLoop newLoop = new ViewLoop();
			        newLoop.ID = j;

			        for (int k = 0; k < oldViews[i].numframes[j]; k++) 
			        {
				        if (oldViews[i].frames[j][k].pic == -1) 
				        {
					        newLoop.RunNextLoop = true;
				        }
				        else 
				        {
					        AGS.Types.ViewFrame newFrame = new AGS.Types.ViewFrame();
					        newFrame.ID = k;
					        newFrame.Flipped = (oldViews[i].frames[j][k].flags & VFLG_FLIPSPRITE);
					        newFrame.Image = oldViews[i].frames[j][k].pic;
					        newFrame.Sound = oldViews[i].frames[j][k].sound;
					        newFrame.Delay = oldViews[i].frames[j][k].speed;

					        newLoop.Frames.Add(newFrame);
				        }
			        }
        			
			        view.Loops.Add(newLoop);
		        }

		        viewFolder.Views.Add(view);
	        }
	        game.RootViewFolder = viewFolder;

	        for (i = 0; i < thisgame.numcharacters; i++) 
	        {
		        char jibbledScriptName[50] = "\0";
		        if (strlen(thisgame.chars[i].scrname) > 0) 
		        {
			        sprintf(jibbledScriptName, "c%s", thisgame.chars[i].scrname);
			        strlwr(jibbledScriptName);
			        jibbledScriptName[1] = toupper(jibbledScriptName[1]);
		        }
		        AGS.Types.Character character = new AGS.Types.Character();
		        character.AdjustSpeedWithScaling = ((thisgame.chars[i].flags & CHF_SCALEMOVESPEED) != 0);
		        character.AdjustVolumeWithScaling = ((thisgame.chars[i].flags & CHF_SCALEVOLUME) != 0);
		        character.AnimationDelay = thisgame.chars[i].animspeed;
		        character.BlinkingView = (thisgame.chars[i].blinkview < 1) ? 0 : (thisgame.chars[i].blinkview + 1);
		        character.Clickable = !(thisgame.chars[i].flags & CHF_NOINTERACT);
		        character.DiagonalLoops = !(thisgame.chars[i].flags & CHF_NODIAGONAL);
		        character.ID = i;
		        character.IdleView = (thisgame.chars[i].idleview < 1) ? 0 : (thisgame.chars[i].idleview + 1);
		        character.MovementSpeed = thisgame.chars[i].walkspeed;
		        character.MovementSpeedX = thisgame.chars[i].walkspeed;
		        character.MovementSpeedY = thisgame.chars[i].walkspeed_y;
		        character.NormalView = thisgame.chars[i].defview + 1;
		        character.RealName = new String(thisgame.chars[i].name);
		        character.ScriptName = new String(jibbledScriptName);
		        character.Solid = !(thisgame.chars[i].flags & CHF_NOBLOCKING);
		        character.SpeechColor = thisgame.chars[i].talkcolor;
		        character.SpeechView = (thisgame.chars[i].talkview < 1) ? 0 : (thisgame.chars[i].talkview + 1);
		        character.StartingRoom = thisgame.chars[i].room;
		        character.StartX = thisgame.chars[i].x;
		        character.StartY = thisgame.chars[i].y;
            character.ThinkingView = (thisgame.chars[i].thinkview < 1) ? 0 : (thisgame.chars[i].thinkview + 1);
		        character.TurnBeforeWalking = !(thisgame.chars[i].flags & CHF_NOTURNING);
		        character.UniformMovementSpeed = (thisgame.chars[i].walkspeed_y == UNIFORM_WALK_SPEED);
		        character.UseRoomAreaLighting = !(thisgame.chars[i].flags & CHF_NOLIGHTING);
		        character.UseRoomAreaScaling = !(thisgame.chars[i].flags & CHF_MANUALSCALING);

		        game.Characters.Add(character);

		        ConvertCustomProperties(character.Properties, &thisgame.charProps[i]);

		        char scriptFuncPrefix[100];
		        sprintf(scriptFuncPrefix, "character%d_", i);
		        ConvertInteractions(character.Interactions, thisgame.intrChar[i], new String(scriptFuncPrefix), game, 3);
	        }
	        game.PlayerCharacter = game.Characters[thisgame.playercharacter];

	        game.TextParser.Words.Clear();
	        for (i = 0; i < thisgame.dict.num_words; i++) 
	        {
		        AGS.Types.TextParserWord newWord = new AGS.Types.TextParserWord();
		        newWord.WordGroup = thisgame.dict.wordnum[i];
		        newWord.Word = new String(thisgame.dict.word[i]);
		        newWord.SetWordTypeFromGroup();

		        game.TextParser.Words.Add(newWord);
	        }

	        for (i = 0; i < MAXGLOBALMES; i++) 
	        {
		        if (thisgame.messages[i] != NULL) 
		        {
			        game.GlobalMessages[i] = new String(thisgame.messages[i]);
		        }
		        else
		        {
			        game.GlobalMessages[i] = String.Empty;
		        }
	        }

	        game.LipSync.Type = (thisgame.options[OPT_LIPSYNCTEXT] != 0) ? LipSyncType.Text : LipSyncType.None;
	        game.LipSync.DefaultFrame = thisgame.default_lipsync_frame;
	        for (i = 0; i < MAXLIPSYNCFRAMES; i++) 
	        {
		        game.LipSync.CharactersPerFrame[i] = new String(thisgame.lipSyncFrameLetters[i]);
	        }

	        for (i = 0; i < thisgame.numdialog; i++) 
	        {
		        AGS.Types.Dialog newDialog = new AGS.Types.Dialog();
		        newDialog.ID = i;
		        for (int j = 0; j < dialog[i].numoptions; j++) 
		        {
			        AGS.Types.DialogOption newOption = new AGS.Types.DialogOption();
			        newOption.ID = j + 1;
			        newOption.Text = new String(dialog[i].optionnames[j]);
			        newOption.Say = !(dialog[i].optionflags[j] & DFLG_NOREPEAT);
			        newOption.Show = (dialog[i].optionflags[j] & DFLG_ON);

			        newDialog.Options.Add(newOption);
		        }

		        newDialog.Name = new String(thisgame.dialogScriptNames[i]);
		        newDialog.Script = new String(dlgscript[i]);
		        newDialog.ShowTextParser = (dialog[i].topicFlags & DTFLG_SHOWPARSER);

		        game.Dialogs.Add(newDialog);
	        }

	        for (i = 0; i < thisgame.numcursors; i++)
	        {
		        AGS.Types.MouseCursor cursor = new AGS.Types.MouseCursor();
		        cursor.Animate = (thisgame.mcurs[i].view >= 0);
		        cursor.AnimateOnlyOnHotspots = ((thisgame.mcurs[i].flags & MCF_HOTSPOT) != 0);
		        cursor.AnimateOnlyWhenMoving = ((thisgame.mcurs[i].flags & MCF_ANIMMOVE) != 0);
		        cursor.Image = thisgame.mcurs[i].pic;
		        cursor.HotspotX = thisgame.mcurs[i].hotx;
		        cursor.HotspotY = thisgame.mcurs[i].hoty;
		        cursor.ID = i;
		        cursor.Name = new String(thisgame.mcurs[i].name);
		        cursor.StandardMode = ((thisgame.mcurs[i].flags & MCF_STANDARD) != 0);
		        cursor.View = thisgame.mcurs[i].view + 1;
		        if (cursor.View < 1) cursor.View = 0;

		        game.Cursors.Add(cursor);
	        }

	        for (i = 0; i < thisgame.numfonts; i++) 
	        {
		        AGS.Types.Font font = new AGS.Types.Font();
		        font.ID = i;
		        font.OutlineFont = (thisgame.fontoutline[i] >= 0) ? thisgame.fontoutline[i] : 0;
		        if (thisgame.fontoutline[i] == -1) 
		        {
			        font.OutlineStyle = FontOutlineStyle.None;
		        }
		        else if (thisgame.fontoutline[i] == FONT_OUTLINE_AUTO)
		        {
			        font.OutlineStyle = FontOutlineStyle.Automatic;
		        }
		        else 
		        {
			        font.OutlineStyle = FontOutlineStyle.UseOutlineFont;
		        }
		        font.PointSize = thisgame.fontflags[i] & FFLG_SIZEMASK;
		        font.Name = new String(String.Format("Font {0}", i));

		        game.Fonts.Add(font);
	        }

	        for (i = 1; i < thisgame.numinvitems; i++)
	        {
		        InventoryItem invItem = new InventoryItem();
            invItem.CursorImage = thisgame.invinfo[i].pic;
		        invItem.Description = new String(thisgame.invinfo[i].name);
		        invItem.Image = thisgame.invinfo[i].pic;
		        invItem.HotspotX = thisgame.invinfo[i].hotx;
		        invItem.HotspotY = thisgame.invinfo[i].hoty;
		        invItem.ID = i;
		        invItem.Name = new String(thisgame.invScriptNames[i]);
		        invItem.PlayerStartsWithItem = (thisgame.invinfo[i].flags & IFLG_STARTWITH);

		        ConvertCustomProperties(invItem.Properties, &thisgame.invProps[i]);

		        char scriptFuncPrefix[100];
		        sprintf(scriptFuncPrefix, "inventory%d_", i);
		        ConvertInteractions(invItem.Interactions, thisgame.intrInv[i], new String(scriptFuncPrefix), game, 5);

		        game.InventoryItems.Add(invItem);
	        }

	        //AGS.Types.CustomPropertySchema schema = new AGS.Types.CustomPropertySchema();
	        for (i = 0; i < thisgame.propSchema.numProps; i++) 
	        {
		        CustomPropertySchemaItem schemaItem = new CustomPropertySchemaItem();
		        schemaItem.Name = new String(thisgame.propSchema.propName[i]);
		        schemaItem.Description = new String(thisgame.propSchema.propDesc[i]);
		        schemaItem.DefaultValue = new String(thisgame.propSchema.defaultValue[i]);
		        schemaItem.Type = (AGS.Types.CustomPropertyType)thisgame.propSchema.propType[i];

		        game.PropertySchema.PropertyDefinitions.Add(schemaItem);
	        }

	        for (i = 0; i < thisgame.numgui; i++)
	        {
		        guis[i].rebuild_array();
	            guis[i].resort_zorder();

		        GUI newGui;
		        if (guis[i].is_textwindow()) 
		        {
			        newGui = new TextWindowGUI();
			        newGui.Controls.Clear();  // we'll add our own edges
              ((TextWindowGUI)newGui).TextColor = guis[i].fgcol;
		        }
		        else 
		        {
			        newGui = new NormalGUI();
			        ((NormalGUI)newGui).Clickable = ((guis[i].flags & GUIF_NOCLICK) == 0);
			        ((NormalGUI)newGui).Top = guis[i].y;
			        ((NormalGUI)newGui).Left = guis[i].x;
			        ((NormalGUI)newGui).Width = (guis[i].wid > 0) ? guis[i].wid : 1;
			        ((NormalGUI)newGui).Height = (guis[i].hit > 0) ? guis[i].hit : 1;
			        ((NormalGUI)newGui).PopupYPos = guis[i].popupyp;
			        ((NormalGUI)newGui).Visibility = (GUIVisibility)guis[i].popup;
			        ((NormalGUI)newGui).ZOrder = guis[i].zorder;
			        ((NormalGUI)newGui).OnClick = new String(guis[i].clickEventHandler);
              ((NormalGUI)newGui).BorderColor = guis[i].fgcol;
		        }
		        newGui.BackgroundColor = guis[i].bgcol;
		        newGui.BackgroundImage = guis[i].bgpic;
		        newGui.ID = i;
		        newGui.Name = new String(guis[i].get_objscript_name(guis[i].name));

		        for (int j = 0; j < guis[i].numobjs; j++)
		        {
			        GUIObject* curObj = guis[i].objs[j];
			        GUIControl newControl = nullptr;
			        switch (guis[i].objrefptr[j] >> 16)
			        {
			        case GOBJ_BUTTON:
				        {
				        if (guis[i].is_textwindow())
				        {
					        AGS.Types.GUITextWindowEdge edge = new AGS.Types.GUITextWindowEdge();
					        .GUIButton *copyFrom = (.GUIButton*)curObj;
					        newControl = edge;
					        edge.Image = copyFrom.pic;
				        }
				        else
				        {
					        AGS.Types.GUIButton newButton = new AGS.Types.GUIButton();
					        .GUIButton *copyFrom = (.GUIButton*)curObj;
					        newControl = newButton;
					        newButton.TextColor = copyFrom.textcol;
					        newButton.Font = copyFrom.font;
					        newButton.Image = copyFrom.pic;
					        newButton.MouseoverImage = copyFrom.overpic;
					        newButton.PushedImage = copyFrom.pushedpic;
					        newButton.TextAlignment = (TextAlignment)copyFrom.textAlignment;
					        newButton.ClickAction = (GUIClickAction)copyFrom.leftclick;
					        newButton.NewModeNumber = copyFrom.lclickdata;
					        newButton.ClipImage = (copyFrom.flags & GUIF_CLIP) ? true : false;
					        newButton.Text = new String(copyFrom.text);
					        newButton.OnClick = new String(copyFrom.eventHandlers[0]);
				        }
				        break;
				        }
			        case GOBJ_LABEL:
				        {
				        AGS.Types.GUILabel newLabel = new AGS.Types.GUILabel();
				        .GUILabel *copyFrom = (.GUILabel*)curObj;
				        newControl = newLabel;
				        newLabel.TextColor = copyFrom.textcol;
				        newLabel.Font = copyFrom.font;
				        newLabel.TextAlignment = (LabelTextAlignment)copyFrom.align;
				        newLabel.Text = new String(copyFrom.GetText());
				        break;
				        }
			        case GOBJ_TEXTBOX:
				        {
				          AGS.Types.GUITextBox newTextbox = new AGS.Types.GUITextBox();
				          .GUITextBox *copyFrom = (.GUITextBox*)curObj;
				          newControl = newTextbox;
				          newTextbox.TextColor = copyFrom.textcol;
				          newTextbox.Font = copyFrom.font;
				          newTextbox.ShowBorder = (copyFrom.exflags & GTF_NOBORDER) ? false : true;
				          newTextbox.Text = new String(copyFrom.text);
				          newTextbox.OnActivate = new String(copyFrom.eventHandlers[0]);
				          break;
				        }
			        case GOBJ_LISTBOX:
				        {
				          AGS.Types.GUIListBox newListbox = new AGS.Types.GUIListBox();
				          .GUIListBox *copyFrom = (.GUIListBox*)curObj;
				          newControl = newListbox;
				          newListbox.TextColor = copyFrom.textcol;
				          newListbox.Font = copyFrom.font; 
				          newListbox.SelectedTextColor = copyFrom.backcol;
				          newListbox.SelectedBackgroundColor = copyFrom.selectedbgcol;
				          newListbox.TextAlignment = (ListBoxTextAlignment)copyFrom.alignment;
				          newListbox.ShowBorder = ((copyFrom.exflags & GLF_NOBORDER) == 0);
				          newListbox.ShowScrollArrows = ((copyFrom.exflags & GLF_NOARROWS) == 0);
				          newListbox.OnSelectionChanged = new String(copyFrom.eventHandlers[0]);
				          break;
				        }
			        case GOBJ_SLIDER:
				        {
				          AGS.Types.GUISlider newSlider = new AGS.Types.GUISlider();
				          .GUISlider *copyFrom = (.GUISlider*)curObj;
				          newControl = newSlider;
				          newSlider.MinValue = copyFrom.min;
				          newSlider.MaxValue = copyFrom.max;
				          newSlider.Value = copyFrom.value;
				          newSlider.HandleImage = copyFrom.handlepic;
			  	          newSlider.HandleOffset = copyFrom.handleoffset;
				          newSlider.BackgroundImage = copyFrom.bgimage;
				          newSlider.OnChange = new String(copyFrom.eventHandlers[0]);
				          break;
				        }
			        case GOBJ_INVENTORY:
				        {
					        AGS.Types.GUIInventory invwindow = new AGS.Types.GUIInventory();
				            .GUIInv *copyFrom = (.GUIInv*)curObj;
				            newControl = invwindow;
					        invwindow.CharacterID = copyFrom.charId;
					        invwindow.ItemWidth = copyFrom.itemWidth;
					        invwindow.ItemHeight = copyFrom.itemHeight;
					        break;
				        }
			        default:
				        throw new AGSEditorException("Unknown control type found: " + (guis[i].objrefptr[j] >> 16));
			        }
			        newControl.Width = (curObj.wid > 0) ? curObj.wid : 1;
			        newControl.Height = (curObj.hit > 0) ? curObj.hit : 1;
			        newControl.Left = curObj.x;
			        newControl.Top = curObj.y;
			        newControl.ZOrder = curObj.zorder;
			        newControl.ID = j;
			        newControl.Name = new String(curObj.scriptName);
			        newGui.Controls.Add(newControl);
		        }
        		
		        game.GUIs.Add(newGui);
	        }

	        free_old_game_data();

	        return game;
        }

        System.String load_room_script(System.String fileName)
        {
	        char roomFileNameBuffer[MAX_PATH];
	        ConvertStringToCharArray(fileName, roomFileNameBuffer);

	        FILE *opty = clibfopen(roomFileNameBuffer, "rb");
	        if (opty == NULL) throw new AGSEditorException("Unable to open room file");

	        short version = getshort(opty);
	        if (version < 17)
	        {
            fclose(opty);
		        throw new AGSEditorException("Room file is from an old version of AGS and cannot be processed");
	        }

	        String scriptToReturn = nullptr;

	        int thisblock = 0;
	        while (thisblock != BLOCKTYPE_EOF) 
	        {
		        thisblock = fgetc(opty);
		        if (thisblock == BLOCKTYPE_EOF) 
		        {
			        break;
		        }

		        int blockLen = getw(opty);

		        if (thisblock == BLOCKTYPE_SCRIPT) 
		        {
			        int lee = getw(opty);
			        int hh;

			        char *scriptFile = (char *)malloc(lee + 5);
			        fread(scriptFile, sizeof(char), lee, opty);
			        scriptFile[lee] = 0;

			        for (hh = 0; hh < lee; hh++)
			          scriptFile[hh] += passwencstring[hh % 11];

			        scriptToReturn = new String(scriptFile, 0, strlen(scriptFile), System.Text.Encoding.Default);
			        free(scriptFile);
			        break;
		        }
		        else 
		        {
			        fseek(opty, blockLen, SEEK_CUR);
		        }
	        }

	        fclose(opty);

	        return scriptToReturn;
        }

        int GetCurrentlyLoadedRoomNumber()
        {
          return loaded_room_number;
        }

        AGS.Types.Room load_crm_file(UnloadedRoom roomToLoad)
        {
	        char roomFileNameBuffer[MAX_PATH];
	        ConvertStringToCharArray(roomToLoad.FileName, roomFileNameBuffer);

	        const char *errorMsg = load_room_file(roomFileNameBuffer);
	        if (errorMsg != NULL) 
	        {
		        throw new AGSEditorException(new String(errorMsg));
	        }

          loaded_room_number = roomToLoad.Number;

	        Room room = new Room(roomToLoad.Number);
	        room.Description = roomToLoad.Description;
	        room.Script = roomToLoad.Script;
	        room.BottomEdgeY = thisroom.bottom;
	        room.LeftEdgeX = thisroom.left;
	        room.MusicVolumeAdjustment = (RoomVolumeAdjustment)thisroom.options[ST_VOLUME];
	        room.PlayerCharacterView = thisroom.options[ST_MANVIEW];
	        room.PlayMusicOnRoomLoad = thisroom.options[ST_TUNE];
	        room.RightEdgeX = thisroom.right;
	        room.SaveLoadEnabled = (thisroom.options[ST_SAVELOAD] == 0);
	        room.ShowPlayerCharacter = (thisroom.options[ST_MANDISABLED] == 0);
	        room.TopEdgeY = thisroom.top;
	        room.Width = thisroom.width;
	        room.Height = thisroom.height;
          if (thisroom.resolution > 2)
          {
            room.Resolution = RoomResolution.HighRes;
          }
          else
          {
  	        room.Resolution = (RoomResolution)thisroom.resolution;
          }
	        room.ColorDepth = bitmap_color_depth(thisroom.ebscene[0]);
	        room.BackgroundAnimationDelay = thisroom.bscene_anim_speed;
	        room.BackgroundCount = thisroom.num_bscenes;

	        int i;
	        for (i = 0; i < thisroom.numLocalVars; i++)
	        {
		        OldInteractionVariable intVar;
		        intVar = new OldInteractionVariable(new String(thisroom.localvars[i].name), thisroom.localvars[i].value);
		        room.OldInteractionVariables.Add(intVar);
	        }

	        for (i = 0; i < thisroom.nummes; i++) 
	        {
		        RoomMessage newMessage = new RoomMessage(i);
		        newMessage.Text = new String(thisroom.message[i]);
		        newMessage.ShowAsSpeech = (thisroom.msgi[i].displayas > 0);
		        newMessage.CharacterID = (thisroom.msgi[i].displayas - 1);
		        newMessage.DisplayNextMessageAfter = ((thisroom.msgi[i].flags & MSG_DISPLAYNEXT) != 0);
		        newMessage.AutoRemoveAfterTime = ((thisroom.msgi[i].flags & MSG_TIMELIMIT) != 0);
		        room.Messages.Add(newMessage);
	        }

	        for (i = 0; i < thisroom.numsprs; i++) 
	        {
		        char jibbledScriptName[50] = "\0";
		        if (strlen(thisroom.objectscriptnames[i]) > 0) 
		        {
			        if (thisroom.wasversion < 26)
			        {
				        sprintf(jibbledScriptName, "o%s", thisroom.objectscriptnames[i]);
				        strlwr(jibbledScriptName);
				        jibbledScriptName[1] = toupper(jibbledScriptName[1]);
			        }
			        else 
			        {			
				        strcpy(jibbledScriptName, thisroom.objectscriptnames[i]);
			        }
		        }

		        RoomObject obj = new RoomObject(room);
		        obj.ID = i;
		        obj.Image = thisroom.sprs[i].sprnum;
		        obj.StartX = thisroom.sprs[i].x;
		        obj.StartY = thisroom.sprs[i].y;
            if (thisroom.wasversion <= 26)
              obj.StartY += GetSpriteHeight(thisroom.sprs[i].sprnum);
		        obj.Visible = (thisroom.sprs[i].on != 0);
		        obj.Baseline = thisroom.objbaseline[i];
		        obj.Name = new String(jibbledScriptName);
		        obj.Description = new String(thisroom.objectnames[i]);
		        obj.UseRoomAreaScaling = ((thisroom.objectFlags[i] & OBJF_USEROOMSCALING) != 0);
		        obj.UseRoomAreaLighting = ((thisroom.objectFlags[i] & OBJF_USEREGIONTINTS) != 0);
		        ConvertCustomProperties(obj.Properties, &thisroom.objProps[i]);

		        if (thisroom.wasversion < 26)
		        {
			        char scriptFuncPrefix[100];
			        sprintf(scriptFuncPrefix, "object%d_", i);
			        ConvertInteractions(obj.Interactions, thisroom.intrObject[i], new String(scriptFuncPrefix), nullptr, 2);
		        }
		        else 
		        {
			        CopyInteractions(obj.Interactions, thisroom.objectScripts[i]);
		        }

		        room.Objects.Add(obj);
	        }

	        for (i = 0; i < thisroom.numhotspots; i++) 
	        {
		        RoomHotspot hotspot = room.Hotspots[i];
		        hotspot.ID = i;
		        hotspot.Description = new String(thisroom.hotspotnames[i]);
		        hotspot.Name = (new String(thisroom.hotspotScriptNames[i])).Trim();
		        hotspot.WalkToPoint = Point(thisroom.hswalkto[i].x, thisroom.hswalkto[i].y);
		        ConvertCustomProperties(hotspot.Properties, &thisroom.hsProps[i]);

		        if (thisroom.wasversion < 26)
		        {
			        char scriptFuncPrefix[100];
			        sprintf(scriptFuncPrefix, "hotspot%d_", i);
			        ConvertInteractions(hotspot.Interactions, thisroom.intrHotspot[i], new String(scriptFuncPrefix), nullptr, 1);
		        }
		        else 
		        {
			        CopyInteractions(hotspot.Interactions, thisroom.hotspotScripts[i]);
		        }
	        }

	        for (i = 0; i <= MAX_WALK_AREAS; i++) 
	        {
		        RoomWalkableArea area = room.WalkableAreas[i];
		        area.ID = i;
		        area.AreaSpecificView = thisroom.shadinginfo[i];
		        area.UseContinuousScaling = !(thisroom.walk_area_zoom2[i] == NOT_VECTOR_SCALED);
		        area.ScalingLevel = thisroom.walk_area_zoom[i] + 100;
		        area.MinScalingLevel = thisroom.walk_area_zoom[i] + 100;
		        if (area.UseContinuousScaling) 
		        {
			        area.MaxScalingLevel = thisroom.walk_area_zoom2[i] + 100;
		        }
		        else
		        {
			        area.MaxScalingLevel = area.MinScalingLevel;
		        }
	        }

	        for (i = 0; i < MAX_OBJ; i++) 
	        {
		        RoomWalkBehind area = room.WalkBehinds[i];
		        area.ID = i;
		        area.Baseline = thisroom.objyval[i];
	        }

	        for (i = 0; i < MAX_REGIONS; i++) 
	        {
		        RoomRegion area = room.Regions[i];
		        area.ID = i;
		        area.UseColourTint = ((thisroom.regionTintLevel[i] & TINT_IS_ENABLED) != 0);
		        area.LightLevel = thisroom.regionLightLevel[i] + 100;
		        area.BlueTint = (thisroom.regionTintLevel[i] >> 16) & 0x00ff;
		        area.GreenTint = (thisroom.regionTintLevel[i] >> 8) & 0x00ff;
		        area.RedTint = thisroom.regionTintLevel[i] & 0x00ff;
		        area.TintSaturation = (thisroom.regionLightLevel[i] > 0) ? thisroom.regionLightLevel[i] : 50;

		        if (thisroom.wasversion < 26)
		        {
			        char scriptFuncPrefix[100];
			        sprintf(scriptFuncPrefix, "region%d_", i);
			        ConvertInteractions(area.Interactions, thisroom.intrRegion[i], new String(scriptFuncPrefix), nullptr, 0);
		        }
		        else 
		        {
			        CopyInteractions(area.Interactions, thisroom.regionScripts[i]);
		        }
	        }
        /*
	        if (thisroom.scripts != NULL) 
	        {
		        room.Script.Text = new String(thisroom.scripts);
	        }
        */
	        room._roomStructPtr = (IntPtr)&thisroom;

	        ConvertCustomProperties(room.Properties, &thisroom.roomProps);

	        if (thisroom.wasversion < 26)
	        {
		        ConvertInteractions(room.Interactions, thisroom.intrRoom, "room_", nullptr, 0);
	        }
	        else 
	        {
		        CopyInteractions(room.Interactions, thisroom.roomScripts);
	        }

	        room.GameID = thisroom.gameId;
          clear_undo_buffer();

	        return room;
        }

        void save_crm_file(Room room)
        {
	        thisroom.gameId = room.GameID;
	        thisroom.bottom = room.BottomEdgeY;
	        thisroom.left = room.LeftEdgeX;
	        thisroom.options[ST_VOLUME] = (int)room.MusicVolumeAdjustment;
	        thisroom.options[ST_MANVIEW] = room.PlayerCharacterView;
	        thisroom.options[ST_TUNE] = room.PlayMusicOnRoomLoad;
	        thisroom.right = room.RightEdgeX;
	        thisroom.options[ST_SAVELOAD] = room.SaveLoadEnabled ? 0 : 1;
	        thisroom.options[ST_MANDISABLED] = room.ShowPlayerCharacter ? 0 : 1;
	        thisroom.top = room.TopEdgeY;
	        thisroom.width = room.Width;
	        thisroom.height = room.Height;
	        thisroom.resolution = (int)room.Resolution;
	        thisroom.bscene_anim_speed = room.BackgroundAnimationDelay;
	        thisroom.num_bscenes = room.BackgroundCount;

	        int i;
	        for (i = 0; i < thisroom.nummes; i++) 
	        {
		        free(thisroom.message[i]);
	        }
	        thisroom.nummes = room.Messages.Count;
	        for (i = 0; i < thisroom.nummes; i++) 
	        {
		        RoomMessage newMessage = room.Messages[i];
		        thisroom.message[i] = (char*)malloc(newMessage.Text.Length + 1);
		        ConvertStringToCharArray(newMessage.Text, thisroom.message[i]);
		        if (newMessage.ShowAsSpeech)
		        {
			        thisroom.msgi[i].displayas = newMessage.CharacterID + 1;
		        }
		        else
		        {
			        thisroom.msgi[i].displayas = 0;
		        }
		        thisroom.msgi[i].flags = 0;
		        if (newMessage.DisplayNextMessageAfter) thisroom.msgi[i].flags |= MSG_DISPLAYNEXT;
		        if (newMessage.AutoRemoveAfterTime) thisroom.msgi[i].flags |= MSG_TIMELIMIT;
	        }

	        thisroom.numsprs = room.Objects.Count;
	        for (i = 0; i < thisroom.numsprs; i++) 
	        {
		        RoomObject obj = room.Objects[i];
		        ConvertStringToCharArray(obj.Name, thisroom.objectscriptnames[i]);

		        thisroom.sprs[i].sprnum = obj.Image;
		        thisroom.sprs[i].x = obj.StartX;
		        thisroom.sprs[i].y = obj.StartY;
		        thisroom.sprs[i].on = obj.Visible;
		        thisroom.objbaseline[i] = obj.Baseline;
		        ConvertStringToCharArray(obj.Description, thisroom.objectnames[i], 30);
		        thisroom.objectFlags[i] = 0;
		        if (obj.UseRoomAreaScaling) thisroom.objectFlags[i] |= OBJF_USEROOMSCALING;
		        if (obj.UseRoomAreaLighting) thisroom.objectFlags[i] |= OBJF_USEREGIONTINTS;
		        CompileCustomProperties(obj.Properties, &thisroom.objProps[i]);
	        }

	        thisroom.numhotspots = room.Hotspots.Count;
	        for (i = 0; i < thisroom.numhotspots; i++) 
	        {
		        RoomHotspot hotspot = room.Hotspots[i];
		        thisroom.hotspotnames[i] = (char*)malloc(hotspot.Description.Length + 1);
		        ConvertStringToCharArray(hotspot.Description, thisroom.hotspotnames[i]);
		        ConvertStringToCharArray(hotspot.Name, thisroom.hotspotScriptNames[i], 20);
		        thisroom.hswalkto[i].x = hotspot.WalkToPoint.X;
		        thisroom.hswalkto[i].y = hotspot.WalkToPoint.Y;
		        CompileCustomProperties(hotspot.Properties, &thisroom.hsProps[i]);
	        }

	        for (i = 0; i <= MAX_WALK_AREAS; i++) 
	        {
		        RoomWalkableArea area = room.WalkableAreas[i];
		        thisroom.shadinginfo[i] = area.AreaSpecificView;
        		
		        if (area.UseContinuousScaling) 
		        {
			        thisroom.walk_area_zoom[i] = area.MinScalingLevel - 100;
			        thisroom.walk_area_zoom2[i] = area.MaxScalingLevel - 100;
		        }
		        else
		        {
			        thisroom.walk_area_zoom[i] = area.ScalingLevel - 100;
			        thisroom.walk_area_zoom2[i] = NOT_VECTOR_SCALED;
		        }
	        }

	        for (i = 0; i < MAX_OBJ; i++) 
	        {
		        RoomWalkBehind area = room.WalkBehinds[i];
		        thisroom.objyval[i] = area.Baseline;
	        }

	        for (i = 0; i < MAX_REGIONS; i++) 
	        {
		        RoomRegion area = room.Regions[i];
		        thisroom.regionTintLevel[i] = 0;
		        if (area.UseColourTint) 
		        {
			        thisroom.regionTintLevel[i] = TINT_IS_ENABLED;
			        thisroom.regionTintLevel[i] |= area.RedTint | (area.GreenTint << 8) | (area.BlueTint << 16);
			        thisroom.regionLightLevel[i] = area.TintSaturation;
		        }
		        else 
		        {
			        thisroom.regionLightLevel[i] = area.LightLevel - 100;
		        }
	        }

	        CompileCustomProperties(room.Properties, &thisroom.roomProps);

	        thisroom.scripts = NULL;
	        thisroom.compiled_script = ((AGS.Native.CompiledScript)room.Script.CompiledData).Data;

	        char roomFileNameBuffer[MAX_PATH];
	        ConvertStringToCharArray(room.FileName, roomFileNameBuffer);

	        TempDataStorage.RoomBeingSaved = room;

	        save_room_file(roomFileNameBuffer);

	        TempDataStorage.RoomBeingSaved = nullptr;

	        for (i = 0; i < thisroom.numhotspots; i++) 
	        {
		        free(thisroom.hotspotnames[i]);
		        thisroom.hotspotnames[i] = NULL;
	        }
        }

        static int CountViews(ViewFolder folder) 
        {
	        int highestViewNumber = 0;
	        for each (ViewFolder subFolder in folder.SubFolders)
	        {
		        int folderView = CountViews(subFolder);
		        if (folderView > highestViewNumber) 
		        {
			        highestViewNumber = folderView;
		        }
	        }
	        for each (View view in folder.Views)
	        {
		        if (view.ID > highestViewNumber)
		        {
			        highestViewNumber = view.ID;
		        }
	        }
	        return highestViewNumber;
        }

        static void ConvertViewsToDTAFormat(ViewFolder folder, Game game) 
        {
	        for each (ViewFolder subFolder in folder.SubFolders)
	        {
		        ConvertViewsToDTAFormat(subFolder, game);
	        }

	        for each (View view in folder.Views)
	        {
		        int i = view.ID - 1;
		        ConvertStringToCharArray(view.Name, thisgame.viewNames[i], MAXVIEWNAMELENGTH);

		        newViews[i].Initialize(view.Loops.Count);
		        for (int j = 0; j < newViews[i].numLoops; j++) 
		        {
              newViews[i].loops[j].Initialize(view.Loops[j].Frames.Count);
			        for (int k = 0; k < newViews[i].loops[j].numFrames; k++) 
			        {
				        AGS.Types.ViewFrame frame = view.Loops[j].Frames[k];
                int frameSound = -1;
                if (frame.Sound > 0) 
                  frameSound = game.GetAudioArrayIndexFromAudioClipIndex(frame.Sound);

				        newViews[i].loops[j].frames[k].flags = (frame.Flipped) ? VFLG_FLIPSPRITE : 0;
				        newViews[i].loops[j].frames[k].pic = frame.Image;
                newViews[i].loops[j].frames[k].sound = frameSound;
				        newViews[i].loops[j].frames[k].speed = frame.Delay;
			        }
        			
			        if (view.Loops[j].RunNextLoop)
			        {
                newViews[i].loops[j].flags = LOOPFLAG_RUNNEXTLOOP;
			        }
		        }

	        }
        }

        void write_compiled_script(FILE *ooo, Script script)
        {
	        if (script.CompiledData == nullptr)
	        {
		        throw new CompileError(String.Format("Script has not been compiled: {0}", script.FileName));
	        }

	        fwrite_script(((AGS.Native.CompiledScript)script.CompiledData).Data, ooo);
        }

        void serialize_interaction_scripts(Interactions interactions, FILE *ooo)
        {
	        char textBuffer[256];
	        putw(interactions.ScriptFunctionNames.Length, ooo);
	        for each (String funcName in interactions.ScriptFunctionNames)
	        {
		        if (funcName == nullptr)
		        {
			        fputc(0, ooo);
		        }
		        else 
		        {
			        ConvertStringToCharArray(funcName, textBuffer, 256);
			        fputstring(textBuffer, ooo);
		        }
	        }
        }

        void serialize_room_interactions(FILE *ooo) 
        {
	        Room roomBeingSaved = TempDataStorage.RoomBeingSaved;
	        serialize_interaction_scripts(roomBeingSaved.Interactions, ooo);
	        for each (RoomHotspot hotspot in roomBeingSaved.Hotspots) 
	        {
		        serialize_interaction_scripts(hotspot.Interactions, ooo);
	        }
	        for each (RoomObject obj in roomBeingSaved.Objects) 
	        {
		        serialize_interaction_scripts(obj.Interactions, ooo);
	        }
	        for each (RoomRegion region in roomBeingSaved.Regions) 
	        {
		        serialize_interaction_scripts(region.Interactions, ooo);
	        }
        }

        void save_thisgame_to_file(const char *fileName, Game game)
        {
	        const char *AGS_VERSION = "3.2.0";
          char textBuffer[500];
	        int bb;

	        FILE*ooo = fopen(fileName, "wb");
	        if (ooo == NULL) 
	        {
		        throw new CompileError(String.Format("Cannot open file {0} for writing", new String(fileName)));
	        }

          fwrite(game_file_sig,30,1,ooo);
          putw(42, ooo);
          putw(strlen(AGS_VERSION), ooo);
          fwrite(AGS_VERSION, strlen(AGS_VERSION), 1, ooo);

          fwrite(&thisgame, sizeof (GameSetupStructBase), 1, ooo);
          fwrite(&thisgame.guid[0], 1, MAX_GUID_LENGTH, ooo);
          fwrite(&thisgame.saveGameFileExtension[0], 1, MAX_SG_EXT_LENGTH, ooo);
          fwrite(&thisgame.saveGameFolderName[0], 1, MAX_SG_FOLDER_LEN, ooo);
          fwrite(&thisgame.fontflags[0], 1, thisgame.numfonts, ooo);
          fwrite(&thisgame.fontoutline[0], 1, thisgame.numfonts, ooo);
          putw (MAX_SPRITES, ooo);
          fwrite(&thisgame.spriteflags[0], 1, MAX_SPRITES, ooo);
          fwrite(&thisgame.invinfo[0], sizeof(InventoryItemInfo), thisgame.numinvitems, ooo);
          fwrite(&thisgame.mcurs[0], sizeof(.MouseCursor), thisgame.numcursors, ooo);
          
          for (bb = 0; bb < thisgame.numcharacters; bb++)
          {
            serialize_interaction_scripts(game.Characters[bb].Interactions, ooo);
          }
          for (bb = 1; bb < thisgame.numinvitems; bb++)
          {
            serialize_interaction_scripts(game.InventoryItems[bb - 1].Interactions, ooo);
          }

          if (thisgame.dict != NULL) {
            write_dictionary (thisgame.dict, ooo);
          }

          write_compiled_script(ooo, game.ScriptsToCompile.GetScriptByFilename(Script.GLOBAL_SCRIPT_FILE_NAME));
          write_compiled_script(ooo, game.ScriptsToCompile.GetScriptByFilename(Script.DIALOG_SCRIPTS_FILE_NAME));

          // Extract all the scripts we want to persist (all the non-headers, except
          // the global script which was already written)
          List<AGS.Types.Script> scriptsToWrite = new List<AGS.Types.Script>();
          for each (Script script in game.ScriptsToCompile)
          {
	          if ((!script.IsHeader) && 
              (!script.FileName.Equals(Script.GLOBAL_SCRIPT_FILE_NAME)) &&
              (!script.FileName.Equals(Script.DIALOG_SCRIPTS_FILE_NAME)))
	          {
		          scriptsToWrite.Add(script);
	          }
          }

          putw(scriptsToWrite.Count, ooo);

          for each (Script script in scriptsToWrite)
          {
	          write_compiled_script(ooo, script);
          }

          for (bb = 0; bb < thisgame.numviews; bb++)
          {
            newViews[bb].WriteToFile(ooo);
          }

          fwrite(&thisgame.chars[0],sizeof(CharacterInfo),thisgame.numcharacters,ooo);

          fwrite(&thisgame.lipSyncFrameLetters[0][0], MAXLIPSYNCFRAMES, 50, ooo);

          char *buffer;
          for (bb=0;bb<MAXGLOBALMES;bb++) 
          {
	          if ((game.GlobalMessages[bb] == nullptr) || (game.GlobalMessages[bb].Length == 0))
		          continue;
              
	          buffer = (char*)malloc(game.GlobalMessages[bb].Length + 1);
	          ConvertStringToCharArray(game.GlobalMessages[bb], buffer);
            write_string_encrypt(ooo, buffer);
	          free(buffer);
          }
          fwrite(&dialog[0], sizeof(DialogTopic), thisgame.numdialog, ooo);
          write_gui(ooo,&guis[0],&thisgame);
          write_plugins_to_disk(ooo);
          // write the custom properties & schema
          thisgame.propSchema.Serialize(ooo);
          for (bb = 0; bb < thisgame.numcharacters; bb++)
            thisgame.charProps[bb].Serialize (ooo);
          for (bb = 0; bb < thisgame.numinvitems; bb++)
            thisgame.invProps[bb].Serialize (ooo);

          for (bb = 0; bb < thisgame.numviews; bb++)
            fputstring(thisgame.viewNames[bb], ooo);

          for (bb = 0; bb < thisgame.numinvitems; bb++)
            fputstring(thisgame.invScriptNames[bb], ooo);

          for (bb = 0; bb < thisgame.numdialog; bb++)
            fputstring(thisgame.dialogScriptNames[bb], ooo);


          int audioClipTypeCount = game.AudioClipTypes.Count + 1;
          putw(audioClipTypeCount, ooo);
          .AudioClipType *clipTypes = (.AudioClipType*)calloc(audioClipTypeCount, sizeof(.AudioClipType));
          // hard-coded SPEECH audio type 0
          clipTypes[0].id = 0;
          clipTypes[0].reservedChannels = 1;
          clipTypes[0].volume_reduction_while_speech_playing = 0;
          clipTypes[0].crossfadeSpeed = 0;

          for (bb = 1; bb < audioClipTypeCount; bb++)
          {
            clipTypes[bb].id = bb;
            clipTypes[bb].reservedChannels = game.AudioClipTypes[bb - 1].MaxChannels;
            clipTypes[bb].volume_reduction_while_speech_playing = game.AudioClipTypes[bb - 1].VolumeReductionWhileSpeechPlaying;
            clipTypes[bb].crossfadeSpeed = (int)game.AudioClipTypes[bb - 1].CrossfadeClips;
          }
          fwrite(clipTypes, sizeof(.AudioClipType), audioClipTypeCount, ooo);
          free(clipTypes);

          IList<AudioClip> allClips = game.CachedAudioClipListForCompile;
          putw(allClips.Count, ooo);
          ScriptAudioClip *compiledAudioClips = (ScriptAudioClip*)calloc(allClips.Count, sizeof(ScriptAudioClip));
          for (int i = 0; i < allClips.Count; i++)
          {
            AudioClip clip = allClips[i];
            ConvertStringToCharArray(clip.ScriptName, compiledAudioClips[i].scriptName, 30);
	          ConvertStringToCharArray(clip.CacheFileNameWithoutPath, compiledAudioClips[i].fileName, 15);
            compiledAudioClips[i].bundlingType = (int)clip.BundlingType;
            compiledAudioClips[i].defaultPriority = (int)clip.ActualPriority;
            compiledAudioClips[i].defaultRepeat = (clip.ActualRepeat) ? 1 : 0;
            compiledAudioClips[i].defaultVolume = clip.ActualVolume;
            compiledAudioClips[i].fileType = (int)clip.FileType;
            compiledAudioClips[i].type = clip.Type;
          }
          fwrite(compiledAudioClips, sizeof(ScriptAudioClip), allClips.Count, ooo);
          free(compiledAudioClips);
          putw(game.GetAudioArrayIndexFromAudioClipIndex(game.Settings.PlaySoundOnScore), ooo);

          if (game.Settings.DebugMode)
          {
            putw(game.Rooms.Count, ooo);
            for (bb = 0; bb < game.Rooms.Count; bb++)
            {
              IRoom room = game.Rooms[bb];
              putw(room.Number, ooo);
              if (room.Description != nullptr)
              {
                ConvertStringToCharArray(room.Description, textBuffer, 500);
              }
              else
              {
                textBuffer[0] = 0;
              }
              fputstring(textBuffer, ooo);
            }
          }

          fclose(ooo);
        }

        void save_game_to_dta_file(Game game, const char *fileName)
        {
	        thisgame.options[OPT_ALWAYSSPCH] = game.Settings.AlwaysDisplayTextAsSpeech;
	        thisgame.options[OPT_ANTIALIASFONTS] = game.Settings.AntiAliasFonts;
	        thisgame.options[OPT_ANTIGLIDE] = game.Settings.AntiGlideMode;
	        thisgame.options[OPT_NOWALKMODE] = !game.Settings.AutoMoveInWalkMode;
	        thisgame.options[OPT_RIGHTLEFTWRITE] = game.Settings.BackwardsText;
	        thisgame.color_depth = (int)game.Settings.ColorDepth;
	        thisgame.options[OPT_COMPRESSSPRITES] = game.Settings.CompressSprites;
	        thisgame.options[OPT_CROSSFADEMUSIC] = (int)game.Settings.CrossfadeMusic;
	        thisgame.options[OPT_DEBUGMODE] = game.Settings.DebugMode;
	        thisgame.options[OPT_DIALOGUPWARDS] = game.Settings.DialogOptionsBackwards;
	        thisgame.options[OPT_DIALOGGAP] = game.Settings.DialogOptionsGap;
	        thisgame.options[OPT_DIALOGIFACE] = game.Settings.DialogOptionsGUI;
	        thisgame.dialog_bullet = game.Settings.DialogOptionsBullet;
	        thisgame.options[OPT_DUPLICATEINV] = game.Settings.DisplayMultipleInventory;
	        thisgame.options[OPT_STRICTSTRINGS] = game.Settings.EnforceNewStrings;
	        thisgame.options[OPT_STRICTSCRIPTING] = game.Settings.EnforceObjectBasedScript;
	        thisgame.options[OPT_NOSCALEFNT] = game.Settings.FontsForHiRes;
	        ConvertStringToCharArray(game.Settings.GameName, thisgame.gamename, 50);
	        thisgame.options[OPT_NEWGUIALPHA] = (int)game.Settings.GUIAlphaStyle;
	        thisgame.options[OPT_HANDLEINVCLICKS] = game.Settings.HandleInvClicksInScript;
	        thisgame.options[OPT_FIXEDINVCURSOR] = !game.Settings.InventoryCursors;
          thisgame.options[OPT_OLDTALKANIMSPD] = game.Settings.LegacySpeechAnimationSpeed;
	        thisgame.options[OPT_LEFTTORIGHTEVAL] = game.Settings.LeftToRightPrecedence;
	        thisgame.options[OPT_LETTERBOX] = game.Settings.LetterboxMode;
          thisgame.totalscore = game.Settings.MaximumScore;
	        thisgame.options[OPT_MOUSEWHEEL] = game.Settings.MouseWheelEnabled;
	        thisgame.options[OPT_DIALOGNUMBERED] = game.Settings.NumberDialogOptions;
	        thisgame.options[OPT_PIXPERFECT] = game.Settings.PixelPerfect;
	        thisgame.options[OPT_SCORESOUND] = 0; // saved elsewhere now to make it 32-bit
	        thisgame.default_resolution = (int)game.Settings.Resolution;
	        thisgame.options[OPT_FADETYPE] = (int)game.Settings.RoomTransition;
	        thisgame.options[OPT_RUNGAMEDLGOPTS] = game.Settings.RunGameLoopsWhileDialogOptionsDisplayed;
	        thisgame.options[OPT_SAVESCREENSHOT] = game.Settings.SaveScreenshots;
	        thisgame.options[OPT_NOSKIPTEXT] = (int)game.Settings.SkipSpeech;
	        thisgame.options[OPT_PORTRAITSIDE] = (int)game.Settings.SpeechPortraitSide;
	        thisgame.options[OPT_SPEECHTYPE] = (int)game.Settings.SpeechStyle;
	        thisgame.options[OPT_SPLITRESOURCES] = game.Settings.SplitResources;
	        thisgame.options[OPT_TWCUSTOM] = game.Settings.TextWindowGUI;
	        thisgame.options[OPT_THOUGHTGUI] = game.Settings.ThoughtGUI;
	        thisgame.options[OPT_TURNTOFACELOC] = game.Settings.TurnBeforeFacing;
	        thisgame.options[OPT_ROTATECHARS] = game.Settings.TurnBeforeWalking;
	        thisgame.options[OPT_NATIVECOORDINATES] = !game.Settings.UseLowResCoordinatesInScript;
	        thisgame.options[OPT_WALKONLOOK] = game.Settings.WalkInLookMode;
	        thisgame.options[OPT_DISABLEOFF] = (int)game.Settings.WhenInterfaceDisabled;
	        thisgame.uniqueid = game.Settings.UniqueID;
          ConvertStringToCharArray(game.Settings.GUIDAsString, thisgame.guid, MAX_GUID_LENGTH);
          ConvertStringToCharArray(game.Settings.SaveGameFolderName, thisgame.saveGameFolderName, MAX_SG_FOLDER_LEN);

          if (game.Settings.EnhancedSaveGames)
          {
            ConvertStringToCharArray(game.Settings.SaveGameFileExtension, thisgame.saveGameFileExtension, MAX_SG_EXT_LENGTH);
          }
          else 
          {
            thisgame.saveGameFileExtension[0] = 0;
          }

	        thisgame.hotdot = game.Settings.InventoryHotspotMarker.DotColor;
	        thisgame.hotdotouter = game.Settings.InventoryHotspotMarker.CrosshairColor;
	        thisgame.invhotdotsprite = game.Settings.InventoryHotspotMarker.Image;
	        if (game.Settings.InventoryHotspotMarker.Style == InventoryHotspotMarkerStyle.Crosshair)
	        {
		        thisgame.invhotdotsprite = 0;
	        }
	        else if (game.Settings.InventoryHotspotMarker.Style == InventoryHotspotMarkerStyle.None)
	        {
		        thisgame.invhotdotsprite = 0;
		        thisgame.hotdot = 0;
	        }

	        // ** Palette **
	        int i;
	        for (i = 0; i < 256; i++) 
	        {
		        if (game.Palette[i].ColourType == PaletteColourType.Background) 
		        {
			        thisgame.paluses[i] = PAL_BACKGROUND;
		        }
		        else 
		        {
			        thisgame.paluses[i] = PAL_GAMEWIDE;
		        }
		        palette[i].r = game.Palette[i].Colour.R / 4;
		        palette[i].g = game.Palette[i].Colour.G / 4;
		        palette[i].b = game.Palette[i].Colour.B / 4;
	        }

	        // ** Plugins **
	        if (game.Plugins.Count > MAX_PLUGINS) 
	        {
		        throw new CompileError("Too many plugins");
	        }

	        numThisgamePlugins = game.Plugins.Count;
	        for (i = 0; i < game.Plugins.Count; i++) 
	        {
		        AGS.Types.Plugin plugin = game.Plugins[i];
		        ConvertStringToCharArray(plugin.FileName, thisgamePlugins[i].filename, 50);
        		
		        thisgamePlugins[i].dataSize = plugin.SerializedData.Length;
		        for (int j = 0; j < thisgamePlugins[i].dataSize; j++) 
		        {
			        thisgamePlugins[i].data[j] = plugin.SerializedData[j];
		        }
	        }

	        // ** Views **
	        int viewCount = CountViews(game.RootViewFolder);

	        thisgame.numviews = viewCount;
          allocate_memory_for_views(viewCount);
          numNewViews = viewCount;

	        ConvertViewsToDTAFormat(game.RootViewFolder, game);

	        // ** Characters **
	        thisgame.numcharacters = game.Characters.Count;
	        thisgame.chars = (CharacterInfo*)calloc(sizeof(CharacterInfo) * thisgame.numcharacters, 1);
          thisgame.charProps = (.CustomProperties*)calloc(thisgame.numcharacters, sizeof(.CustomProperties));
	        for (i = 0; i < thisgame.numcharacters; i++) 
	        {
		        AGS.Types.Character character = game.Characters[i];
		        thisgame.chars[i].flags = 0;
		        thisgame.chars[i].on = 1;
		        if (character.AdjustSpeedWithScaling) thisgame.chars[i].flags |= CHF_SCALEMOVESPEED;
		        if (character.AdjustVolumeWithScaling) thisgame.chars[i].flags |= CHF_SCALEVOLUME;
		        if (!character.Clickable) thisgame.chars[i].flags |= CHF_NOINTERACT;
		        if (!character.DiagonalLoops) thisgame.chars[i].flags |= CHF_NODIAGONAL;
            if (character.MovementLinkedToAnimation) thisgame.chars[i].flags |= CHF_ANTIGLIDE;
		        if (!character.Solid) thisgame.chars[i].flags |= CHF_NOBLOCKING;
		        if (!character.TurnBeforeWalking) thisgame.chars[i].flags |= CHF_NOTURNING;
		        if (!character.UseRoomAreaLighting) thisgame.chars[i].flags |= CHF_NOLIGHTING;
		        if (!character.UseRoomAreaScaling) thisgame.chars[i].flags |= CHF_MANUALSCALING;

		        if (character.UniformMovementSpeed) 
		        {
			        thisgame.chars[i].walkspeed = character.MovementSpeed;
			        thisgame.chars[i].walkspeed_y = UNIFORM_WALK_SPEED;
		        }
		        else 
		        {
			        thisgame.chars[i].walkspeed = character.MovementSpeedX;
			        thisgame.chars[i].walkspeed_y = character.MovementSpeedY;
		        }

		        thisgame.chars[i].animspeed = character.AnimationDelay;
		        thisgame.chars[i].blinkview = character.BlinkingView - 1;
		        thisgame.chars[i].idleview = character.IdleView - 1;
		        thisgame.chars[i].defview = character.NormalView - 1;
		        thisgame.chars[i].view = thisgame.chars[i].defview;
		        ConvertStringToCharArray(character.RealName, thisgame.chars[i].name, 40);
		        ConvertStringToCharArray(character.ScriptName, thisgame.chars[i].scrname, MAX_SCRIPT_NAME_LEN);
		        thisgame.chars[i].talkcolor = character.SpeechColor;
		        thisgame.chars[i].talkview = character.SpeechView - 1;
		        thisgame.chars[i].room = character.StartingRoom;
            thisgame.chars[i].speech_anim_speed = character.SpeechAnimationDelay;
		        thisgame.chars[i].x = character.StartX;
		        thisgame.chars[i].y = character.StartY;
		        thisgame.chars[i].thinkview = character.ThinkingView - 1;

		        CompileCustomProperties(character.Properties, &thisgame.charProps[i]);
	        }
	        thisgame.playercharacter = game.PlayerCharacter.ID;

	        // ** Text Parser **
          thisgame.dict = (WordsDictionary*)malloc(sizeof(WordsDictionary));
          thisgame.dict.allocate_memory(game.TextParser.Words.Count);
          for (i = 0; i < thisgame.dict.num_words; i++)
	        {
            ConvertStringToCharArray(game.TextParser.Words[i].Word, thisgame.dict.word[i], MAX_PARSER_WORD_LENGTH);
            thisgame.dict.wordnum[i] = game.TextParser.Words[i].WordGroup;
	        }

	        // ** Global Messages **
	        for (i = 0; i < MAXGLOBALMES; i++) 
	        {
		        if (game.GlobalMessages[i] != String.Empty) 
		        {
			        thisgame.messages[i] = (char*)malloc(game.GlobalMessages[i].Length + 1);
			        ConvertStringToCharArray(game.GlobalMessages[i], thisgame.messages[i]);
		        }
		        else
		        {
			        thisgame.messages[i] = NULL;
		        }
	        }

	        // ** Lip Sync **
	        if (game.LipSync.Type == LipSyncType.Text)
	        {
		        thisgame.options[OPT_LIPSYNCTEXT] = 1;
	        }
	        else 
	        {
		        thisgame.options[OPT_LIPSYNCTEXT] = 0;
	        }
	        thisgame.default_lipsync_frame = game.LipSync.DefaultFrame;
	        for (i = 0; i < MAXLIPSYNCFRAMES; i++) 
	        {
		        ConvertStringToCharArray(game.LipSync.CharactersPerFrame[i], thisgame.lipSyncFrameLetters[i], 50);
	        }

	        // ** Dialogs **
	        if (game.Dialogs.Count > MAX_DIALOG) 
	        {
		        throw new CompileError("Too many dialogs");
	        }
	        thisgame.numdialog = game.Dialogs.Count;
	        thisgame.numdlgmessage = 0;
	        dialog = (DialogTopic*)malloc(sizeof(DialogTopic) * thisgame.numdialog);
	        for (i = 0; i < thisgame.numdialog; i++) 
	        {
		        Dialog curDialog = game.Dialogs[i];
		        dialog[i].numoptions = curDialog.Options.Count;
		        for (int j = 0; j < dialog[i].numoptions; j++) 
		        {
			        AGS.Types.DialogOption option = curDialog.Options[j];
			        ConvertStringToCharArray(option.Text, dialog[i].optionnames[j], 150);
			        //dialog[i].entrypoints[j] = option.EntryPointOffset;
			        dialog[i].optionflags[j] = 0;
			        if (!option.Say) 
			        {
				        dialog[i].optionflags[j] |= DFLG_NOREPEAT;
			        }
			        if (option.Show)
			        {
				        dialog[i].optionflags[j] |= DFLG_ON;
			        }
		        }
        		
            dialog[i].optionscripts = NULL;
        /*	    dialog[i].optionscripts = (unsigned char*)malloc(curDialog.CodeSize);
	            Marshal.Copy(curDialog.CompiledCode, 0, IntPtr(dialog[i].optionscripts), curDialog.CodeSize);

		        dialog[i].codesize = curDialog.CodeSize;
		        dialog[i].startupentrypoint = curDialog.StartupEntryPointOffset;

		        dlgscript[i] = (char*)malloc(curDialog.Script.Length + 1);
		        ConvertStringToCharArray(curDialog.Script, dlgscript[i]);*/

		        ConvertStringToCharArray(curDialog.Name, thisgame.dialogScriptNames[i], MAX_SCRIPT_NAME_LEN);

		        dialog[i].topicFlags = 0;
		        if (curDialog.ShowTextParser) dialog[i].topicFlags |= DTFLG_SHOWPARSER;
	        }

	        // ** Cursors **
	        if (game.Cursors.Count > MAX_CURSOR) 
	        {
		        throw new CompileError("Too many cursors");
	        }
	        thisgame.numcursors = game.Cursors.Count;
	        for (i = 0; i < thisgame.numcursors; i++)
	        {
		        AGS.Types.MouseCursor cursor = game.Cursors[i];
		        thisgame.mcurs[i].flags = 0;
		        if (cursor.Animate) 
		        {
			        thisgame.mcurs[i].view = cursor.View - 1;
			        if (cursor.AnimateOnlyOnHotspots) thisgame.mcurs[i].flags |= MCF_HOTSPOT;
			        if (cursor.AnimateOnlyWhenMoving) thisgame.mcurs[i].flags |= MCF_ANIMMOVE;
		        }
		        else 
		        {
			        thisgame.mcurs[i].view = -1;
		        }
		        if (cursor.StandardMode) thisgame.mcurs[i].flags |= MCF_STANDARD;
		        thisgame.mcurs[i].pic = cursor.Image;
		        thisgame.mcurs[i].hotx = cursor.HotspotX;
		        thisgame.mcurs[i].hoty = cursor.HotspotY;
		        ConvertStringToCharArray(cursor.Name, thisgame.mcurs[i].name, 10);
	        }

	        // ** Fonts **
	        if (game.Fonts.Count > MAX_FONTS)
	        {
		        throw new CompileError("Too many fonts");
	        }
	        thisgame.numfonts = game.Fonts.Count;
	        for (i = 0; i < thisgame.numfonts; i++) 
	        {
		        AGS.Types.Font font = game.Fonts[i];
		        thisgame.fontflags[i] = font.PointSize & FFLG_SIZEMASK;
		        if (font.OutlineStyle == FontOutlineStyle.None) 
		        {
			        thisgame.fontoutline[i] = -1;
		        }
		        else if (font.OutlineStyle == FontOutlineStyle.Automatic) 
		        {
			        thisgame.fontoutline[i] = FONT_OUTLINE_AUTO;
		        }
		        else
		        {
			        thisgame.fontoutline[i] = font.OutlineFont;
		        }
	        }

	        // ** Inventory items **
	        if (game.InventoryItems.Count > MAX_INV)
	        {
		        throw new CompileError("Too many inventory items");
	        }
	        thisgame.numinvitems = game.InventoryItems.Count + 1;
	        for (i = 1; i < thisgame.numinvitems; i++)
	        {
		        InventoryItem invItem = game.InventoryItems[i - 1];
		        ConvertStringToCharArray(invItem.Description, thisgame.invinfo[i].name, 25);
		        ConvertStringToCharArray(invItem.Name, thisgame.invScriptNames[i], MAX_SCRIPT_NAME_LEN);
		        thisgame.invinfo[i].pic = invItem.Image; 
            thisgame.invinfo[i].cursorPic = invItem.CursorImage;
		        thisgame.invinfo[i].hotx = invItem.HotspotX;
		        thisgame.invinfo[i].hoty = invItem.HotspotY;
		        thisgame.invinfo[i].flags = 0;
		        if (invItem.PlayerStartsWithItem) thisgame.invinfo[i].flags |= IFLG_STARTWITH;

		        CompileCustomProperties(invItem.Properties, &thisgame.invProps[i]);
	        }

	        // ** Custom Properties Schema **
	        List<AGS.Types.CustomPropertySchemaItem > schema = game.PropertySchema.PropertyDefinitions;
	        if (schema.Count > MAX_CUSTOM_PROPERTIES)
	        {
		        throw new CompileError("Too many custom properties defined");
	        }
	        thisgame.propSchema.numProps = schema.Count;
	        for (i = 0; i < thisgame.propSchema.numProps; i++) 
	        {
		        CustomPropertySchemaItem schemaItem = schema[i];
		        ConvertStringToCharArray(schemaItem.Name, thisgame.propSchema.propName[i], 20);
		        ConvertStringToCharArray(schemaItem.Description, thisgame.propSchema.propDesc[i], 100);
		        thisgame.propSchema.defaultValue[i] = (char*)malloc(schemaItem.DefaultValue.Length + 1);
		        ConvertStringToCharArray(schemaItem.DefaultValue, thisgame.propSchema.defaultValue[i]);
		        thisgame.propSchema.propType[i] = (int)schemaItem.Type;
	        }

	        // ** GUIs **
	        numguibuts = 0;
	        numguilabels = 0;
	        numguitext = 0;
	        numguilist = 0;
	        numguislider = 0;
	        numguiinv = 0;

	        thisgame.numgui = game.GUIs.Count;
          guis = (GUIMain*)calloc(thisgame.numgui, sizeof(GUIMain));
	        for (i = 0; i < thisgame.numgui; i++)
	        {
		        guis[i].init();
		        ConvertGUIToBinaryFormat(game.GUIs[i], &guis[i]);
		        guis[i].highlightobj = -1;
	        }

	        // this cannot be null, so set it randomly
	        thisgame.compiled_script = (ccScript*)0x12345678;

	        save_thisgame_to_file(fileName, game);
        	
	        free_old_game_data();
        }



#endif


        // 5142: end
    }
}
