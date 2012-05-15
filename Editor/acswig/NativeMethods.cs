using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Drawing;
using AGS.Types;

namespace AGS.Native
{
	public class NativeMethods
	{
#if false
        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr GetModuleHandle(string lpModuleName);
#endif

        private string _editorVersionNumber;

        public NativeMethods(String version)
        {
            _editorVersionNumber = version;
        }

        public void Initialize()
        {
            if (!acswig.initialize_native())
	        {
		        throw new InvalidDataException("Native initialization failed.");
	        }
#if false
            IntPtr hnd = GetModuleHandle(null);
            if (!acswig.Scintilla_RegisterClasses(hnd))
            {
                throw new InvalidDataException("Native initialization failed.");
            }
#endif
        }

        public void NewGameLoaded(Game game)
        {
            this.PaletteColoursUpdated(game);
            NativeMethodsUtil.GameUpdated(game);
            NativeMethodsUtil.UpdateSpriteFlags(game.RootSpriteFolder);
        }

        public void PaletteColoursUpdated(Game game)
        {
            //lastPaletteSet = game.Palette;
            NativeMethodsUtil.PaletteUpdated(game.Palette);
        }

        public void LoadNewSpriteFile()
        {
            if (!acswig.reset_sprite_file())
            {
                throw new AGSEditorException("Unable to load the sprite file ACSPRSET.SPR. The file may be missing, corrupt or it may require a newer version of AGS.");
            }
        }
        public Bitmap GetBitmapForSprite(int spriteSlot, int width, int height)
        {
            return NativeMethodsUtil.getSpriteAsBitmap32bit(spriteSlot, width, height);
        }
        public Bitmap GetBitmapForSpritePreserveColDepth(int spriteSlot)
        {
            return NativeMethodsUtil.getSpriteAsBitmap(spriteSlot);
        }


        public void SaveGame(Game game)
		{
			NativeMethodsUtil.save_game(game.Settings.CompressSprites);
		}

        public void GameSettingsChanged(Game game)
		{
			NativeMethodsUtil.GameUpdated(game);
		}

        public void DrawGUI(int hDC, int x, int y, GUI gui, int scaleFactor, int selectedControl)
		{
			NativeMethodsUtil.drawGUI(hDC, x, y, gui, scaleFactor, selectedControl);
		}

        public void DrawSprite(int hDC, int x, int y, int spriteNum, bool flipImage)
		{
			acswig.drawSprite(hDC, x, y, spriteNum, flipImage);
		}

        public void DrawFont(int hDC, int x, int y, int fontNum)
		{
			acswig.drawFontAt(hDC, fontNum, x, y);
		}

        public void DrawSprite(int hDC, int x, int y, int width, int height, int spriteNum)
		{
			acswig.drawSpriteStretch(hDC, x, y, width, height, spriteNum);
		}

        public void DrawBlockOfColour(int hDC, int x, int y, int width, int height, int colourNum)
		{
			acswig.drawBlockOfColour(hDC, x, y, width, height, colourNum);
		}

        public void DrawViewLoop(int hdc, ViewLoop loopToDraw, int x, int y, int size, int cursel)
		{
			NativeMethodsUtil.drawViewLoop(hdc, loopToDraw, x, y, size, cursel);
		}

        public bool DoesSpriteExist(int spriteNumber)
        {
			if ((spriteNumber < 0) || (spriteNumber >= acswig.GetMaxSprites()))
			{
				return false;
			}
			return acswig.DoesSpriteExist(spriteNumber);
        }

        public void ImportSCIFont(String fileName, int fontSlot) 
		{
            //var fileNameBuf = new byte[acswig.MAX_PATH];
            //NativeMethodsUtil.NativeMethodsUtil.ConvertFileNameToCharArray(fileName, fileNameBuf);
            var errorMsg = acswig.import_sci_font(fileName, fontSlot);
			if (errorMsg != null) 
			{
				throw new AGSEditorException(errorMsg);
			}
		}

        public void ReloadTTFFont(int fontSlot)
    {
      if (!acswig.reload_font(fontSlot))
      {
        throw new AGSEditorException("Unable to load the TTF font file. The renderer was unable to load the font.");
      }
    }

		// Gets sprite height in 320x200-res co-ordinates
        public int GetRelativeSpriteHeight(int spriteSlot) 
		{
			return acswig.GetRelativeSpriteHeight(spriteSlot);
		}

		// Gets sprite width in 320x200-res co-ordinates
        public int GetRelativeSpriteWidth(int spriteSlot) 
		{
			return acswig.GetRelativeSpriteWidth(spriteSlot);
		}

        public int GetActualSpriteWidth(int spriteSlot) 
		{
			return acswig.GetSpriteWidth(spriteSlot);
		}

        public int GetActualSpriteHeight(int spriteSlot) 
		{
			return acswig.GetSpriteHeight(spriteSlot);
		}

        public int GetSpriteResolutionMultiplier(int spriteSlot)
		{
			return acswig.GetSpriteResolutionMultiplier(spriteSlot);
		}

        public void ChangeSpriteNumber(Sprite sprite, int newNumber)
		{
			if ((newNumber < 0) || (newNumber >= acswig.GetMaxSprites()))
			{
				throw new AGSEditorException("Invalid sprite number");
			}
			acswig.change_sprite_number(sprite.Number, newNumber);
			sprite.Number = newNumber;
		}

        public void SpriteResolutionsChanged(Sprite[] sprites)
		{
			foreach (Sprite sprite in sprites)
			{
				acswig.update_sprite_resolution(sprite.Number, sprite.Resolution == Types.SpriteImportResolution.HighRes);
			}
		}

        public Sprite SetSpriteFromBitmap(int spriteSlot, Bitmap bmp, int spriteImportMethod, bool remapColours, bool useRoomBackgroundColours, bool alphaChannel)
		{
			int spriteRes = NativeMethodsUtil.SetNewSpriteFromBitmap(spriteSlot, bmp, spriteImportMethod, remapColours, useRoomBackgroundColours, alphaChannel);
      int colDepth = acswig.GetSpriteColorDepth(spriteSlot);
			Sprite newSprite = new Sprite(spriteSlot, bmp.Width, bmp.Height, colDepth, (SpriteImportResolution)spriteRes, alphaChannel);
      int roomNumber = NativeMethodsUtil.GetCurrentlyLoadedRoomNumber();
      if ((colDepth == 8) && (useRoomBackgroundColours) && (roomNumber >= 0))
      {
        newSprite.ColoursLockedToRoom = roomNumber;
      }
      return newSprite;
		}

        public void ReplaceSpriteWithBitmap(Sprite spr, Bitmap bmp, int spriteImportMethod, bool remapColours, bool useRoomBackgroundColours, bool alphaChannel)
		{
			int spriteRes = NativeMethodsUtil.SetNewSpriteFromBitmap(spr.Number, bmp, spriteImportMethod, remapColours, useRoomBackgroundColours, alphaChannel);
			spr.Resolution = (SpriteImportResolution)spriteRes;
			spr.ColorDepth = acswig.GetSpriteColorDepth(spr.Number);
			spr.Width = bmp.Width;
			spr.Height = bmp.Height;
			spr.AlphaChannel = alphaChannel;
      spr.ColoursLockedToRoom = 0;
      int roomNumber = NativeMethodsUtil.GetCurrentlyLoadedRoomNumber();
      if ((spr.ColorDepth == 8) && (useRoomBackgroundColours) && (roomNumber >= 0))
      {
        spr.ColoursLockedToRoom = roomNumber;
      }
		}


        public void DeleteSprite(int spriteSlot)
		{
			acswig.deleteSprite(spriteSlot);
		}

        public int GetFreeSpriteSlot()
		{
			return acswig.find_free_sprite_slot();
		}

        public bool CropSpriteEdges(IList<Sprite> sprites, bool symmetric)
		{	
			var spriteSlotList = new int[sprites.Count];
			int i = 0;
			foreach (Sprite sprite in sprites)
			{
				spriteSlotList[i] = sprite.Number;
				i++;
			}
			bool result = acswig.crop_sprite_edges(sprites.Count, spriteSlotList, symmetric) != 0;

			int newWidth = acswig.GetSpriteWidth(sprites[0].Number);
			int newHeight = acswig.GetSpriteHeight(sprites[0].Number);
			foreach (Sprite sprite in sprites)
			{
				sprite.Width = newWidth;
				sprite.Height = newHeight;
			}
			return result;
		}

        public void Shutdown()
		{
			acswig.shutdown_native();
		}

        public Game ImportOldGameFile(String fileName)
		{
			//byte fileNameBuf = new byte[acswig.MAX_PATH];
			//NativeMethodsUtil.ConvertFileNameToCharArray(fileName, fileNameBuf);

            Game game = NativeMethodsUtil.load_old_game_dta_file(fileName);

			return game;
		}

        public Dictionary<int, Sprite> LoadAllSpriteDimensions()
		{
			return NativeMethodsUtil.load_sprite_dimensions();
		}

        public Room LoadRoomFile(UnloadedRoom roomToLoad)
		{
			return NativeMethodsUtil.load_crm_file(roomToLoad);
		}

        public void SaveRoomFile(Room roomToSave)
		{
			NativeMethodsUtil.save_crm_file(roomToSave);
		}

        public void CreateBuffer(int width, int height) 
		{
		 NativeMethodsUtil.CreateBuffer(width, height);
		}

        public void DrawSpriteToBuffer(int sprNum, int x, int y, int scaleFactor) 
		{
		 NativeMethodsUtil.DrawSpriteToBuffer(sprNum, x, y, scaleFactor);
		}

        public void RenderBufferToHDC(int hDC) 
		{
		 NativeMethodsUtil.RenderBufferToHDC(hDC);
		}

        public void DrawRoomBackground(int hDC, Room room, int x, int y, int backgroundNumber, float scaleFactor, RoomAreaMaskType maskType, int selectedArea, int maskTransparency)
		{
			acswig.draw_room_background(room._roomStructPtr, hDC, x, y, backgroundNumber, scaleFactor, (int)maskType, selectedArea, maskTransparency);
		}

        public void ImportBackground(Room room, int backgroundNumber, Bitmap bmp, bool useExactPalette, bool sharePalette)
		{
		 NativeMethodsUtil.ImportBackground(room, backgroundNumber, bmp, useExactPalette, sharePalette);
		}

        public void DeleteBackground(Room room, int backgroundNumber)
		{
		 NativeMethodsUtil.DeleteBackground(room, backgroundNumber);
		}

        public Bitmap GetBitmapForBackground(Room room, int backgroundNumber)
		{
			return NativeMethodsUtil.getBackgroundAsBitmap(room, backgroundNumber);
		}

        public void DrawLineOntoMask(Room room, RoomAreaMaskType maskType, int x1, int y1, int x2, int y2, int color)
		{
			acswig.draw_line_onto_mask(room._roomStructPtr, (int)maskType, x1, y1, x2, y2, color);
		}

        public void DrawFilledRectOntoMask(Room room, RoomAreaMaskType maskType, int x1, int y1, int x2, int y2, int color)
		{
			acswig.draw_filled_rect_onto_mask(room._roomStructPtr, (int)maskType, x1, y1, x2, y2, color);
		}

        public void DrawFillOntoMask(Room room, RoomAreaMaskType maskType, int x1, int y1, int color)
		{
			acswig.draw_fill_onto_mask(room._roomStructPtr, (int)maskType, x1, y1, color);
		}

        public void CopyWalkableMaskToRegions(Room room) 
		{
			acswig.copy_walkable_to_regions(room._roomStructPtr);
		}

        public int GetAreaMaskPixel(Room room, RoomAreaMaskType maskType, int x, int y)
		{
			return acswig.get_mask_pixel(room._roomStructPtr, (int)maskType, x, y);
		}

        public void ImportAreaMask(Room room, RoomAreaMaskType maskType, Bitmap bmp)
    {
      NativeMethodsUtil.import_area_mask(room._roomStructPtr, (int)maskType, bmp);
    }

        public void CreateUndoBuffer(Room room, RoomAreaMaskType maskType)
		{
			acswig.create_undo_buffer(room._roomStructPtr, (int)maskType);
		}

        public bool DoesUndoBufferExist()
		{
			return acswig.does_undo_buffer_exist();
		}

        public void ClearUndoBuffer()
		{
			acswig.clear_undo_buffer();
		}

        public void RestoreFromUndoBuffer(Room room, RoomAreaMaskType maskType)
		{
			acswig.restore_from_undo_buffer(room._roomStructPtr, (int)maskType);
		}

        public void SetGreyedOutMasksEnabled(bool enabled)
    {
      acswig.enable_greyed_out_masks = enabled;
    }

        public String LoadRoomScript(String roomFileName) 
		{
			return NativeMethodsUtil.load_room_script(roomFileName);
		}

        public BaseTemplate LoadTemplateFile(String fileName, bool isRoomTemplate)
    {
                    throw new NotImplementedException();
#if false
      //var fileNameBuf = new byte[acswig.MAX_PATH];
			//NativeMethodsUtil.ConvertFileNameToCharArray(fileName, fileNameBuf);
      char *iconDataBuffer = null;
      long iconDataSize = 0;

      int success = acswig.load_template_file(fileName, &iconDataBuffer, &iconDataSize, isRoomTemplate);
			if (success) 
			{
				Icon icon = null;
				if (iconDataBuffer != null)
				{
          cli::array<unsigned char> managedArray = new cli::array<unsigned char>(iconDataSize);
          Marshal::Copy(IntPtr(iconDataBuffer), managedArray, 0, iconDataSize);
          _global.free(iconDataBuffer);
          System::IO::MemoryStream ms = new System::IO::MemoryStream(managedArray);
          try 
          {
					  icon = new Icon(ms);
          } 
          catch (ArgumentException) 
          {
            // it is not a valid .ICO file, ignore it
            icon = null;
          }
				}
        if (isRoomTemplate)
        {
          return new RoomTemplate(fileName, icon);
        }
        else
        {
				  return new GameTemplate(fileName, icon);
        }
			}
			return null;
#endif
    }

        public GameTemplate LoadTemplateFile(String fileName)
		{
      return (GameTemplate)LoadTemplateFile(fileName, false);
		}

        public RoomTemplate LoadRoomTemplateFile(String fileName)
		{
      return (RoomTemplate)LoadTemplateFile(fileName, true);
		}

        public void ExtractTemplateFiles(String templateFileName) 
		{
            //var fileNameBuf = new byte[acswig.MAX_PATH];
			//NativeMethodsUtil.ConvertFileNameToCharArray(templateFileName, fileNameBuf);

            if (acswig.extract_template_files(templateFileName) != 0)
			{
				throw new AGSEditorException("Unable to extract template files.");
			}
		}

        public void ExtractRoomTemplateFiles(String templateFileName, int newRoomNumber) 
		{
			//var fileNameBuf = new byte[acswig.MAX_PATH];
			//NativeMethodsUtil.ConvertFileNameToCharArray(templateFileName, fileNameBuf);

            if (acswig.extract_room_template_files(templateFileName, newRoomNumber) != 0)
			{
				throw new AGSEditorException("Unable to extract template files.");
			}
		}

        public byte[] TransformStringToBytes(String text) 
		{
            throw new NotImplementedException();
#if false
			char* stringPointer = (char*)Marshal::StringToHGlobalAnsi(text).ToPointer();
			int textLength = text.Length + 1;
			var toReturn = new byte[textLength + 4];
			toReturn[0] = textLength % 256;
			toReturn[1] = textLength / 256;
			toReturn[2] = 0;
			toReturn[3] = 0;
	
			acswig.transform_string(stringPointer);

			{ pin_ptr<unsigned char> nativeBytes = &toReturn[4];
				memcpy(nativeBytes, stringPointer, textLength);
			}

			Marshal::FreeHGlobal(IntPtr(stringPointer));

			return toReturn;
#endif
		}

        public bool HaveSpritesBeenModified()
		{
			return acswig.spritesModified;
		}

    // from ScriptCompiler.cpp
    public void CompileScript(Script script, string[] preProcessedScripts, Game game, bool isRoomScript) { throw new NotImplementedException();}
    public void CompileGameToDTAFile(Game game, String fileName) { throw new NotImplementedException();}
    public void CreateDataFile(string[] fileList, long splitSize, String baseFileName, bool isGameEXE) { throw new NotImplementedException();}
    public void CreateVOXFile(String fileName, string[] fileList) { throw new NotImplementedException();}
    public void UpdateFileIcon(String fileToUpdate, String iconName) { throw new NotImplementedException();}
    public void UpdateGameExplorerThumbnail(String fileToUpdate, byte[] newData) { throw new NotImplementedException();}
    public void UpdateGameExplorerXML(String fileToUpdate, byte[] newData) { throw new NotImplementedException();}
    //public void FindAndUpdateMemory(unsigned char *data, int dataLen, const unsigned char *searchFor, int searchForLen, const unsigned char *replaceWith) { throw new NotImplementedException();}
    //public void ReplaceStringInMemory(unsigned char *memory, int memorySize, const char *searchFor, const unsigned char *replaceWithData) { throw new NotImplementedException();}
    public void UpdateFileVersionInfo(String fileToUpdate, byte[] authorNameUnicode, byte[] gameNameUnicode) { throw new NotImplementedException();}
    public void UpdateResourceInFile(String fileToUpdate, string resourceName, byte[] newData) { throw new NotImplementedException();}
	};

	public class SourceCodeControl
		{
        public SourceCodeControl(){ }
        public bool Initialize(string dllName, int mainWindowHwnd) { return false; }
        public void Shutdown(){  }
        public SourceControlProject AddToSourceControl() { return null; }
        public bool OpenProject(SourceControlProject project) { return false; }
        public void CloseProject(){  }
        public SourceControlFileStatus[] GetFileStatuses(string[] fileNames){ return new SourceControlFileStatus[0]; }
        public void AddFilesToSourceControl(string[] fileNames, string comment) {  }
        public void CheckInFiles(string[] fileNames, string comment) { }
        public void CheckOutFiles(string[] fileNames, string comment) { }
        public void RenameFile(string currentPath, string newPath) { }
        public void DeleteFiles(string[] fileNames, string comment) { }
	};

}
