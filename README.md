#### 数字選択式宝くじ分析プログラム

統計的手法で当選数字を分析する。入出力は基本的にファイルで行うCUIだが、一部GUIあり。
ナンバーズ、ミニロト、ロト6共にプログラムの構成は共通。  
あくまで分析なので当選を保証するものではない。  
2007年に開発し、その後多少のメンテナンスをしつつも長らく非公開だった。

AGPL-3.0 License  
Copyright (C) 2007 kozok-dev

## ビルド

Microsoft Visual C++ Toolkit 2003、Platform SDK等をインストールしてパスを通した後、build.batを実行。

## ana.c

メインプログラム。scr.txt、wn.txt、exp.txt(任意)を読み、分析結果であるana.txtを出力する。  
scr.txtでの設定値によってはtsv形式のana_\*.txtも出力する(ana_mm.txtは例外)。

## anas.bat

過去の分析結果の統計をanas.txtとして出力する。実際はanaを引数付きで実行しているだけ。

## ccn.c

ana_ec.txt、ana_wn.txtを読み、ana_ec.txtの数字を良さそうな順番に並べ替えたexp.txtを出力する。
sqlite形式のexp.dbもおまけで出力される。

## cn.c

wn.txtを読み、限られた数字から全組み合わせを買い続けていた場合の統計をcn.txtとして出力する。  
数字は遺伝的アルゴリズム、または総当たりで自動的に選ばれるが、ロト6のみcne.txtが読めれば手動で選べる。  
遺伝的アルゴリズムの場合は無限ループなので適当な所でEnterを押して終わらせる。

## cwn.\*

exp.txtを読み、GUIで当選確認を行う。

## mn.c

引数で指定されたファイルまたはexp.txtを読み、GUIでシートにマークする箇所を確認する。

## pms.\*

GUIで数字を選び、マークシートに印刷する。うまく印刷されるかは分からない。  
数字をファイルから読み込むことも可。

## pn.c

exp.txtの数字を並び替える。

## wn.c

引数で指定されたcsvファイルから当選数字をwn.txtとして出力する。  
csvはネット上のいろいろな所から取得できるが、それに合わせてプログラムの修正が必要。

## scr.txt

分析に関する設定。詳細は省略するが、scr.cが理解できれば分かるはず。
