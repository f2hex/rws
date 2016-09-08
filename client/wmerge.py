#!/usr/bin/python
"""
-Script to merge together web assets (resources) into a single HTML page for ESP8266 based apps
-input: a main index.html that can contains related linked CSS and JS files
-output: a gzipped html page (index.min.html.gz) and a converted version of it in a 
         "c declaration" format  (index.min-hex.html.h)
 author: Franco Fiorese - 24-aug-2016
"""

from bs4 import BeautifulSoup as bs
from htmlmin import minify as hmin
from jsmin import jsmin as jmin
from csscompressor import compress as cmin
import StringIO
import gzip

# look inside the index.html file for any href resource
soup = bs(open("index.html"))
styles = ""
for link in soup.findAll('link'):
    with open(link["href"], 'rb') as finp:
        styles += cmin(finp.read().strip()) + "\n"
    link.extract()

new_tag = soup.new_tag("style")
new_tag.string = styles
soup.body.insert_before(new_tag)

scripts = ""
for script in soup.findAll('script'):
    with open(script["src"], 'rb') as finp:
        scripts += jmin(finp.read().strip()) + "\n"
    script.extract()

new_tag = soup.new_tag("script")
new_tag.string = scripts
soup.style.insert_after(new_tag)

# minify the resulting HTML content...
minified = hmin(soup.prettify(formatter="html").decode('utf-8'), remove_empty_space=True)

# ...and gzip it
out = StringIO.StringIO()
with gzip.GzipFile(fileobj=out, mode="w") as f:
  f.write(minified)
# save result gzipped HTML as file 
with open("index.min.html.gz",'wb') as fout:
    fout.write(out.getvalue())
