#!/bin/bash

# コマンドライン引数を処理
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <input_file> <output_file>"
    exit 1
fi

input_file="$1"
output_file="$2"

# 入力ファイルの1行目と2行目を出力ファイルにコピー
head -n2 "$input_file" > "$output_file"

# csvヘッダーを追記
echo "type,x,y,z,vx,vy,vz" >> "$output_file"

# profファイルの各行を処理 (3行目から開始)
tail -n +3 "$input_file" | while IFS=" " read -r type x y z vx vy vz; do
    # 空行や数値以外の行をスキップ
    if [[ -z "$type" || ! "$type" =~ ^[0-9]+$ ]]; then
        continue
    fi

    # csvフォーマットで出力
    echo "$type,$x,$y,$z,$vx,$vy,$vz" >> "$output_file"
done
