#! /bin/bash
#
rev_new=$(git rev-parse --short=7 HEAD)

rev_old=$(cat ./src/gitrev.c 2>/dev/null | tr -d '\n' | awk -F"\"" '{print $2}' | awk -F"\"" '{print $1}')


if [ "$rev_new" != "$rev_old" ] || [ ! -f ./src/gitrev.c ]; then

	if [ -n "$rev_new" ]; then
		echo "Changed Rev $rev_old to $rev_new" >&2
	fi

    cat <<EOF > ./src/gitrev.c
#define GIT_REV "$rev_new"

const char *GetRev()
{
	return GIT_REV;
}
EOF

    rev_date=`date -u +%Y%m%d%H%M%S`

    cat <<EOF > ./meta/meta.xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
  <name>Loadiine GX2</name>
  <coder>Dimok, Maschell, n1ghty, dibas</coder>
  <version>0.3 r$rev_new</version>
  <release_date>$rev_date</release_date>
  <short_description>WiiU game loader</short_description>
  <long_description>Loads games from SD card.

  Compatibility list:
  http://wiki.gbatemp.net/wiki/Loadiine_compatibility_list

  Sources:
  https://github.com/dimok789/loadiine_gx2
  </long_description>
</app>
EOF

fi

echo $rev_new
