# M5Stack ATOM Matrix デモ

M5Stack ATOM Matrix（5×5 フルカラー LED、ボタン、傾きセンサー付き）で動くデモです。
デスクトップ版 Claude の Code タブに「このPCにM5Stack ATOM Matrixが接続されているので、何かデモをして。」と頼み、Claude が作って書き込みました。

## 動作の様子

| レインボー | 傾きボール | 文字スクロール | 花火 |
|---|---|---|---|
| ![レインボー](media/rainbow.jpg) | ![傾きボール](media/tilt.gif) | ![文字スクロール](media/text.gif) | ![花火](media/sparkle.jpg) |

傾きボールと文字スクロールは動きが見どころなので、短い GIF にしています。

**動画**（クリックすると再生できます。音声なし）

| レインボー | 傾きボール | 文字スクロール | 花火 |
|---|---|---|---|
| [rainbow.mp4](media/video/rainbow.mp4)（5秒） | [tilt.mp4](media/video/tilt.mp4)（11秒） | [text.mp4](media/video/text.mp4)（13秒） | [sparkle.mp4](media/video/sparkle.mp4)（6秒） |

<!--
写真・動画の追加方法:
1. 撮影した元ファイルは media_raw/ に置く（位置情報を含むので .gitignore で除外している）
2. 位置情報を消して縮小したコピーを media/ に作る
   - 写真: 切り抜いて保存し直す（メタデータは引き継がない）
   - 動画: ffmpeg -i IN.mov -map 0:v:0 -an -map_metadata -1 -vf scale=-2:960 -c:v libx264 -crf 28 -movflags +faststart OUT.mp4
3. README から ![説明](media/xxx.jpg) のように参照する
4. ページ上で再生ボタン付きの動画にしたいときは、GitHub の Web 画面でこの README を編集し、mp4 をドラッグ＆ドロップする
-->

## Claude での操作の流れ

デスクトップ版 Claude の Code タブ（Pro プラン）で、約30分かかりました。人が入力したのは短い文と「許可」だけです。
（画面は会話の部分だけを切り抜いています）

### 1. 一文で頼む

作業用のフォルダを選び、次の一文を入力しただけです。細かい手順は伝えていません。

![一文で頼む](media/claude-ui/01-ask.png)

### 2. Claude が計画を立てる

Claude は「確認 → 作成 → 書き込み → 動作確認」の4段階を自分で決め、つながっている場所（COM3）や使える道具を調べ始めます。

![計画を立てる](media/claude-ui/02-plan.png)

### 3. 大事な操作の前に許可を求めてくる

本体のプログラムの上書きや、道具のダウンロード（容量も書いてある）の前に、必ず確認があります。不安なら「プランモード」で、計画を見てから進めることもできます。

![許可を求める](media/claude-ui/03-permission.png)

### 4. あとは待つだけ

道具の導入、元のプログラムのバックアップ、コンパイルを順に進めます。バックアップが失敗したときは、自分で速度を下げてやり直しました。この間、人は何もしていません。

![作業の進み具合](media/claude-ui/04-progress.png)

### 5. 結果の報告と確認の依頼

書き込みと起動の確認が済むと、遊び方や元に戻す方法まで説明してくれます。Claude には LED が見えないので、見た目は人に確かめるよう頼んできます。

![結果の報告](media/claude-ui/05-report.png)

### 6. 実物の様子を伝えて直してもらう

「どの向きに傾けたら、どう動いたか」を伝えると、Claude はセンサーの値（ログ）で原因を確かめてから直します。

![様子を伝える](media/claude-ui/06-feedback.png)

### 役割分担

| Claude がしたこと | 人がしたこと |
|---|---|
| つながっている機器と道具を調べる | やりたいことを一文で伝える |
| 足りない道具を探して入れる | 上書きとダウンロードを許可する |
| プログラムを書く、直す | LED の光り方や動きを目で見て伝える |
| 本体へ書き込み、起動ログで確認 | どこまでで「良し」とするか決める |
| 元に戻せるようバックアップを取る | |

### つまずきから学んだこと

傾きボールは、1回目は「USB の向きは正しく、直角の向きだけ逆」でした。Claude はその軸だけ向きを反転させましたが、2回目は4方向とも逆になりました。最初の見立てが違っていたためです。今回は「気泡ゲーム」として見ればよいと判断し、修正は後日にしました。

- AI の判断も外れることがある。実物で確かめるまでが一区切り
- 「どうしたら、どうなったか」を具体的に返すと、次の一手が決まる
- 直すか、そのまま使うかは人が決めてよい

### はじめて使う人へのコツ

1. **ざっくり頼んでいい**: 「何かデモをして」でも、Claude が状況を調べてから提案してくれる
2. **大事な操作は聞いてくる**: 上書きやダウンロードの前に確認がある
3. **結果は具体的に返す**: 「どうしたら、どうなったか」を書くと、原因をしぼり込める
4. **続きから再開できる**: プログラム・バックアップ・覚え書き（メモリ）が残るので、後日の修正も続きから

## 4つのモード

LED の面を押すたびに、次のモードに切り替わります。

| # | モード | 内容 |
|---|---|---|
| 0 | レインボー | 中心から虹色が渦を巻いて回る |
| 1 | 傾きボール | 本体を傾けるとボールが転がり、壁で跳ね返る（内蔵 IMU MPU6886 を使用） |
| 2 | 文字スクロール | 「HELLO ATOM M5!」が色を変えながら流れる |
| 3 | 花火 | ランダムな位置で光がはじけて消える |

発熱を抑えるため、LED の明るさは 20/255 にしています。

## 使うピン

| 部品 | ピン |
|---|---|
| LED（WS2812 × 25） | GPIO27 |
| ボタン | GPIO39 |
| IMU（MPU6886） | I2C SDA=GPIO25 / SCL=GPIO21 |

## 手順（Windows）

### 1. 道具を入れる

[arduino-cli](https://github.com/arduino/arduino-cli/releases) を入れたあと、ESP32 のボード定義と NeoPixel ライブラリを入れます。

```bash
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "Adafruit NeoPixel"
```

動作を確かめた版: arduino-cli 1.6.0、esp32:esp32 3.3.12。

### 2. 元のファームウェアを保存する（任意・おすすめ）

書き込むと元のプログラムは上書きされます。先に丸ごと読み出しておくと、あとで戻せます。
esptool は ESP32 のボード定義と一緒に入ります（`%LOCALAPPDATA%\Arduino15\packages\esp32\tools\esptool_py\<版>\esptool.exe`）。

```bash
esptool --port COM3 --baud 115200 read-flash 0 ALL backup_original_firmware.bin
```

4MB の読み出しに約7分かかります。921600 baud では途中で失敗したため、115200 baud を使っています。

### 3. ビルドして書き込む

`COM3` は自分の環境のポート番号に変えてください（デバイスマネージャーの「ポート (COM と LPT)」で分かります）。

```bash
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:m5stack_atom:UploadSpeed=115200 atom_demo
```

### 4. 動作を確かめる

シリアルモニター（115200 baud）に次のように出れば起動しています。

```
IMU WHO_AM_I = 0x19
ATOM Matrix demo start. IMU: OK
Mode 0: Rainbow
```

傾きボールのモードでは、1秒ごとに加速度（x, y, z）が出ます。

### 5. 元のファームウェアに戻す

```bash
esptool --port COM3 --baud 115200 write-flash 0 backup_original_firmware.bin
```

## 分かっている問題

- **傾きボールが4方向とも逆に動く**: 傾けて持ち上がった側へ動くので、今は「気泡」として扱っています。直すときは `modeTilt()` で x 軸・y 軸の両方の符号を反転させ（`velX ... + ax`、`velY ... + ay`）、実機で確かめます。
