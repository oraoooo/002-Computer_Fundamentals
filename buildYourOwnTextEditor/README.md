# buildYourOwnTextEditor (kilo)

Based on [Build Your Own Text Editor](https://viewsourcecode.org/snaptoken/kilo/) —
versi tutorial dari editor `kilo` punya antirez.

## Yang ditambahin: logger ritme ngetik

`editorLogKeystroke()` dipanggil tiap kali ada keypress, nyatet jarak waktu
dari keystroke sebelumnya + kode tombolnya + ditandain kalo itu backspace.
Default-nya mati total (gaada file ditulis) kecuali env var `KILO_KEYLOG`
di-set — jadi kalo dipakai biasa ya perilakunya sama aja kaya tutorial
aslinya.

Idenya: biar editor ini bisa dipake buat ngeliat kebiasaan ngetik sendiri —
kapan ngetik cepet, kapan mikir dulu, seberapa sering kena backspace. Ada
`analyze_keystrokes.py` buat baca log-nya dan ngasih summary + histogram.

## Jalanin

```sh
make
KILO_KEYLOG=/tmp/keys.log ./kilo somefile.txt
```

Ngetik-ngetik, pause, backspace, terus Ctrl-Q **4x** buat keluar (3x
pertama cuma warning "unsaved changes", itu bukan dari gue — perilaku asli
tutorialnya). Terus:

```sh
python3 analyze_keystrokes.py /tmp/keys.log
```

## Test

```sh
make test
```

Nge-drive kilo lewat pty asli (soalnya raw mode butuh TTY beneran, gabisa
cuma di-pipe), ngetik cepet, pause lama sengaja, backspace sekali, terus
ngecek log-nya nyatet pause-nya sama backspace-nya bener.
