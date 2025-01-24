#!/usr/bin/sh

function compilar
{
	if [[ ! -f "$1" ]]; then
		echo "aviso: arquivo '$1' não encontrado"
		return
	fi

	bin=$(basename "$1")
	bin=${bin%.*}

	out=$(gcc "$1" -o "$bin" \
		-ggdb3
	)
	_e=$?
	if [[  $_e != 0 ]]; then
		echo "falhar ao compilar arquivo '$1'"
		echo $out
	else
		echo "sucesso ao compilar arquivo '$1'"
		if [[ -n "$out" ]]; then
			echo $out
		fi
	fi
}

compilar "questao_1.c"
compilar "questao_2.c"
compilar "questao_3.c"
compilar "questao_4.c"
compilar "questao_05.c"
compilar "questao_5_formatada.c"
compilar "questao_6.c"
