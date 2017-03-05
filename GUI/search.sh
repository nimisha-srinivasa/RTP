#!/bin/bash

echo "Content-type: text/html"
echo ""

echo '<!DOCTYPE html>'
echo '<html lang="en">'
echo '<head>'
echo ' <meta charset="utf-8">'
echo '<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=yes">'
echo '<link href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.0/css/bootstrap.min.css" rel="stylesheet">'
echo '<script src="https://code.jquery.com/jquery-1.12.4.min.js" integrity="sha256-ZosEbRLbNQzLpnKIkEdrPv7lOy9C27hHQ+Xp8a4MxAQ=" crossorigin="anonymous"></script>'
echo '<script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.0/js/bootstrap.min.js"></script>'
echo '</head>'
echo '<body>'

echo '<div class="container-fluid">
  <div class="row">
      <div class="col-md-1"></div>
      <div class="col-md-10"></div>
      <div class="col-md-1"></div>
  </div>
 
  <div class="row">
      <div class="col-md-2"></div>
      <div class="col-md-7">
        <h1>RTP Search Results</h1>
      </div>
      <div class="col-md-3"></div>
  </div>
</div>
<br/>'
echo '<div class="container-fluid">
<div class="row">
      <div class="col-md-2"></div>
      <div class="col-md-8">'

QUERY=`echo "$QUERY_STRING" | sed -n -e 's/^.*query=\([^&]*\).*$/\1/p' -e "s/%20/ /g"`
QUERY=`echo "$QUERY" | sed 's/\+/\ /g' `
TOP_K=`echo "$QUERY_STRING" | sed -n -e 's/^.*topk=\([^&]*\).*$/\1/p' -e "s/%20/ /g"`
output=$(./run_single_query.sh "$QUERY" $TOP_K)
echo "<pre>"
echo "$output"
echo "</pre>"

echo '</div>
      <div class="col-md-2"></div>
  </div>'


echo '</body>'
echo '</html>'

exit 0

