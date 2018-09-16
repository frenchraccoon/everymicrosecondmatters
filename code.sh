#!/bin/bash
#

set -ue

# <https://www.algolia.com/realtime-search-demo/code-penal-francais>

# Source:
# <https://www.legifrance.gouv.fr/telecharger_pdf.do?cidTexte=LEGITEXT000006070719>
# converted into text (Acrobat)

declare -A sect
declare -A title
par=
tok=
text=
first=1

sect[Livre]=
sect[Titre]=
sect[Chapitre]=
sect[Section]=
sect[Sous-section]=
sect[Article]=

title[Livre]=
title[Titre]=
title[Chapitre]=
title[Section]=
title[Sous-section]=
title[Article]=

cat << EOF
[
EOF

cat code.txt \
	| tail -n +5 \
	| tr -d '\r\f' \
	| sed -e 's/^Code pénal - Dernière modification.*/#/g' -e 's/^Copyright .*/#/g' \
	| tr '\n' '#' \
	| sed -e 's/########/#/g' \
	| tr '#' '\n' \
	| (
	while read line; do
		match=
		if [[ "$line" =~ ^(Livre|Titre|Chapitre|Section|Sous-section|Article)\ (Ier|[RIVXC0-9\-]*)( :|[[:space:]]*$) ]]; then
			match=1
		fi

		# commit lines
		if [ -n "$match" ] || [ "$line" == "" ]; then
			# title & co.
			if [ -n "$tok" ]; then
				title[$tok]="$par"
				tok=
			else
				par="$(echo "$par"|sed -e 's/"/\\"/g')"
				if [ -n "$text" ]; then
					text="${text}\n${par}"
				else
					text="$par"
				fi

				if [ -n "$match" ] && [ -n "$text" ]; then
					if [ -n "$first" ]; then
						first=
					else
						echo ","
					fi
cat << EOF
  {
  "Livre": "${sect[Livre]}:${title[Livre]}",
  "Titre": "${sect[Titre]}:${title[Titre]}",
EOF
					if [ -n "${sect[Chapitre]}" ]; then
cat << EOF
  "Chapitre": "${sect[Chapitre]}:${title[Chapitre]}",
EOF
					fi
					if [ -n "${sect[Section]}" ]; then
cat << EOF
  "Section": "${sect[Section]}:${title[Section]}",
EOF
					fi
					if [ -n "${sect[Sous-section]}" ]; then
cat << EOF
  "Sous-section": "${sect[Sous-section]}:${title[Sous-section]}",
EOF
					fi
					if [ -n "${sect["Article"]}" ]; then
cat << EOF
  "Article": "${sect[Article]}",
EOF
					fi
cat << EOF
  "Url": "https://www.google.com/search?btnI&q=%22Code+p%C3%A9nal%22+%22${sect[Article]}%22+site%3Alegifrance.gouv.fr",
  "Text": "$text"
  }
EOF
				text=
				fi
			fi
		fi

		if test -n "$match"; then
			tok="$(echo "$line"|cut -f1 -d' ')"
			num="$(echo "$line"|cut -f2 -d' ')"

			# pretty
			num="$(echo "$num"|sed \
				-e 's/Ier/I/' \
				-e 's/^I$/1/' \
				-e 's/^II$/2/' \
				-e 's/^III$/3/' \
				-e 's/^IV$/4/' \
				-e 's/^V$/5/' \
				-e 's/^VI$/6/' \
				-e 's/^VII$/7/' \
				-e 's/^VIII$/8/' \
				-e 's/^IX$/9/' \
				-e 's/^X$/10/' \
				-e 's/^XI$/11/' \
				-e 's/^XII$/12/' \
				-e 's/^XIII$/13/' \
				-e 's/^XIII$/13/' \
				-e 's/^XIV$/14/' \
				-e 's/^XV$/15/' \
				-e 's/^XVI$/16/' \
				-e 's/^XVII$/17/' \
				-e 's/^XVIII$/18/' \
				-e 's/^XIX/19/' \
				-e 's/^XX$/20/' \
			)"

			sect[$tok]="$num"

			par="$(echo "$line"|cut -f3- -d' '|sed -e 's/^[ :]*//g')"

			case $tok in
			Livre)
			sect[Titre]=
			sect[Chapitre]=
			sect[Section]=
			sect[Sous-section]=
			sect[Article]=
			title[Titre]=
			title[Chapitre]=
			title[Section]=
			title[Sous-section]=
			title[Article]=
			;;
			Titre)
			sect[Chapitre]=
			sect[Section]=
			sect[Sous-section]=
			sect[Article]=
			title[Chapitre]=
			title[Section]=
			title[Sous-section]=
			title[Article]=
			;;
			Chapitre)
			sect[Section]=
			sect[Sous-section]=
			sect[Article]=
			title[Section]=
			title[Sous-section]=
			title[Article]=
			;;
			Section)
			sect[Sous-section]=
			sect[Article]=
			title[Sous-section]=
			title[Article]=
			;;
			Sous-section)
			sect[Article]=
			title[Article]=
			;;
			esac
		elif [ "$line" == "" ]; then
			par=
		else
			if test -n "$par"; then
				par="$par $line"
			else
				par="$line"
			fi
		fi
	done
#grep -E "^(Livre|Titre|Chapitre|Section|Sous-section|Article) (Ier|[IVXC0-9\-]*)( :|[[:space:]]*$)" code.txt |more
)

cat << EOF
]
EOF
