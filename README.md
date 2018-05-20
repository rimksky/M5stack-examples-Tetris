## M5Stack examples Tetris

### LICENSE

Original 1 (on ESP32)  
https://github.com/MhageGH/esp32_ILI9328_Tetris.git

Original 2 (porting M5Stack)  
https://macsbug.wordpress.com/2018/01/20/tetris-with-m5stack/

Original 3 (showing next tetris and scoring)  
https://gist.github.com/shikarunochi/2e8af90518e8325cccbd4a697ff113b3#file-tetris-ino

AND more...

### 変更箇所

* M5Stack BASICで動作確認。
* 左ボタンの長押しで「下」に行くようにした。
* 左右の長押しで、連続して左右に動くようにした。
* コンパイルオプションで -DUSE_SD_UPDATER を渡しておくと、SD_UPDATER対応になります。  
  esp32/platform.txt 内の compiler.cpreprocessor.flags=に-DUSE_SD_UPDATERを追加するなど。



