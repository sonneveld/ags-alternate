using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using AGS.Types;

namespace AGS.Native
{
	public class NativeMethods
	{
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
        }

        public void NewGameLoaded(Game game){ throw new NotImplementedException(); }
        public void SaveGame(Game game){ throw new NotImplementedException(); }
        public void GameSettingsChanged(Game game){ throw new NotImplementedException(); }
        public void PaletteColoursUpdated(Game game){ throw new NotImplementedException(); }
        public void DrawGUI(int hDC, int x, int y, GUI gui, int scaleFactor, int selectedControl){ throw new NotImplementedException(); }
        public void DrawSprite(int hDC, int x, int y, int width, int height, int spriteNum){ throw new NotImplementedException(); }
        public void DrawSprite(int hDC, int x, int y, int spriteNum, bool flipImage){ throw new NotImplementedException(); }
        public void DrawFont(int hDC, int x, int y, int fontNum){ throw new NotImplementedException(); }
        public void DrawBlockOfColour(int hDC, int x, int y, int width, int height, int colourNum){ throw new NotImplementedException(); }
        public void DrawViewLoop(int hdc, ViewLoop loopToDraw, int x, int y, int size, int cursel){ throw new NotImplementedException(); }
        public Sprite SetSpriteFromBitmap(int spriteSlot, Bitmap bmp, int spriteImportMethod, bool remapColours, bool useRoomBackgroundColours, bool alphaChannel){ throw new NotImplementedException(); }
        public void ReplaceSpriteWithBitmap(Sprite spr, Bitmap bmp, int spriteImportMethod, bool remapColours, bool useRoomBackgroundColours, bool alphaChannel){ throw new NotImplementedException(); }
        public Bitmap GetBitmapForSprite(int spriteSlot, int width, int height){ throw new NotImplementedException(); }
        public Bitmap GetBitmapForSpritePreserveColDepth(int spriteSlot){ throw new NotImplementedException(); }
        public void DeleteSprite(int spriteSlot){ throw new NotImplementedException(); }
        public int  GetFreeSpriteSlot(){ throw new NotImplementedException(); }
        public int  GetRelativeSpriteWidth(int spriteSlot){ throw new NotImplementedException(); }
        public int  GetRelativeSpriteHeight(int spriteSlot){ throw new NotImplementedException(); }
        public int  GetActualSpriteWidth(int spriteSlot){ throw new NotImplementedException(); }
        public int  GetActualSpriteHeight(int spriteSlot){ throw new NotImplementedException(); }
        public int  GetSpriteResolutionMultiplier(int spriteSlot){ throw new NotImplementedException(); }
        public bool CropSpriteEdges(IList<Sprite> sprites, bool symmetric){ throw new NotImplementedException(); }
        public bool DoesSpriteExist(int spriteNumber){ throw new NotImplementedException(); }
        public void ChangeSpriteNumber(Sprite sprite, int newNumber){ throw new NotImplementedException(); }
        public void SpriteResolutionsChanged(Sprite[] sprites){ throw new NotImplementedException(); }
        public void Shutdown(){ throw new NotImplementedException(); }
        public Game ImportOldGameFile(String fileName){ throw new NotImplementedException(); }
        public void ImportSCIFont(String fileName, int fontSlot){ throw new NotImplementedException(); }
        public void ReloadTTFFont(int fontSlot){ throw new NotImplementedException(); }
        public Dictionary<int,Sprite> LoadAllSpriteDimensions(){ throw new NotImplementedException(); }
        public void LoadNewSpriteFile(){ throw new NotImplementedException(); }
        public Room LoadRoomFile(UnloadedRoom roomToLoad){ throw new NotImplementedException(); }
        public void SaveRoomFile(Room roomToSave){ throw new NotImplementedException(); }
        public void DrawRoomBackground(int hDC, Room room, int x, int y, int backgroundNumber, float scaleFactor, RoomAreaMaskType maskType, int selectedArea, int maskTransparency){ throw new NotImplementedException(); }
        public void ImportBackground(Room room, int backgroundNumber, Bitmap bmp, bool useExactPalette, bool sharePalette){ throw new NotImplementedException(); }
        public void DeleteBackground(Room room, int backgroundNumber){ throw new NotImplementedException(); }
        public Bitmap GetBitmapForBackground(Room room, int backgroundNumber){ throw new NotImplementedException(); }
        public void DrawLineOntoMask(Room room, RoomAreaMaskType maskType, int x1, int y1, int x2, int y2, int color){ throw new NotImplementedException(); }
        public void DrawFilledRectOntoMask(Room room, RoomAreaMaskType maskType, int x1, int y1, int x2, int y2, int color){ throw new NotImplementedException(); }
        public void DrawFillOntoMask(Room room, RoomAreaMaskType maskType, int x1, int y1, int color){ throw new NotImplementedException(); }
        public void CopyWalkableMaskToRegions(Room room){ throw new NotImplementedException(); }
        public int  GetAreaMaskPixel(Room room, RoomAreaMaskType maskType, int x, int y){ throw new NotImplementedException(); }
        public void ImportAreaMask(Room room, RoomAreaMaskType maskType, Bitmap bmp){ throw new NotImplementedException(); }
        public void CreateUndoBuffer(Room room, RoomAreaMaskType maskType){ throw new NotImplementedException(); }
        public bool DoesUndoBufferExist(){ throw new NotImplementedException(); }
        public void ClearUndoBuffer(){ throw new NotImplementedException(); }
        public void RestoreFromUndoBuffer(Room room, RoomAreaMaskType maskType){ throw new NotImplementedException(); }
        public void SetGreyedOutMasksEnabled(bool enabled){ throw new NotImplementedException(); }
        public void CreateBuffer(int width, int height) { throw new NotImplementedException(); }
        public void DrawSpriteToBuffer(int sprNum, int x, int y, int scaleFactor) { throw new NotImplementedException(); }
        public void RenderBufferToHDC(int hDC) { throw new NotImplementedException(); }
        public String LoadRoomScript(String roomFileName){ throw new NotImplementedException(); }
        public void CompileScript(Script script, string[] preProcessedScripts, Game game, bool isRoomScript){ throw new NotImplementedException(); }
        public void CompileGameToDTAFile(Game game, String fileName){ throw new NotImplementedException(); }
        public void CreateDataFile(string[] fileList, long splitSize, String baseFileName, bool isGameEXE){ throw new NotImplementedException(); }
        public void CreateVOXFile(String fileName, string[] fileList){ throw new NotImplementedException(); }
        public GameTemplate LoadTemplateFile(String fileName){ throw new NotImplementedException(); }
        public RoomTemplate LoadRoomTemplateFile(String fileName){ throw new NotImplementedException(); }
        public void ExtractTemplateFiles(String templateFileName){ throw new NotImplementedException(); }
        public void ExtractRoomTemplateFiles(String templateFileName, int newRoomNumber){ throw new NotImplementedException(); }
        public void UpdateFileIcon(String fileToUpdate, String iconFileName){ throw new NotImplementedException(); }
        public void UpdateGameExplorerXML(String fileToUpdate, byte[] data){ throw new NotImplementedException(); }
        public void UpdateGameExplorerThumbnail(String fileToUpdate, byte[] data){ throw new NotImplementedException(); }
        public void UpdateFileVersionInfo(String fileToUpdate, byte[] authorNameUnicode, byte[] gameNameUnicode){ throw new NotImplementedException(); }
        public byte[] TransformStringToBytes(String text){ throw new NotImplementedException(); }
        public bool HaveSpritesBeenModified(){ throw new NotImplementedException(); }
	};

	public class SourceCodeControl
		{
        public SourceCodeControl(){ throw new NotImplementedException(); }
        public bool Initialize(string dllName, int mainWindowHwnd){ throw new NotImplementedException(); }
        public void Shutdown(){ throw new NotImplementedException(); }
        public SourceControlProject AddToSourceControl(){ throw new NotImplementedException(); }
        public bool OpenProject(SourceControlProject project){ throw new NotImplementedException(); }
        public void CloseProject(){ throw new NotImplementedException(); }
        public SourceControlFileStatus[] GetFileStatuses(string[] fileNames){ throw new NotImplementedException(); }
        public void AddFilesToSourceControl(string[] fileNames, string comment) { throw new NotImplementedException(); }
        public void CheckInFiles(string[] fileNames, string comment) { throw new NotImplementedException(); }
        public void CheckOutFiles(string[] fileNames, string comment) { throw new NotImplementedException(); }
        public void RenameFile(string currentPath, string newPath) { throw new NotImplementedException(); }
        public void DeleteFiles(string[] fileNames, string comment) { throw new NotImplementedException(); }
	};

}
