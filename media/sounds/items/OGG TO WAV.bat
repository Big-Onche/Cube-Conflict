FOR %%i IN (*.ogg) DO C:\ffmpeg\bin\ffmpeg -i "%%i" -ac 1 "%%~ni.wav"