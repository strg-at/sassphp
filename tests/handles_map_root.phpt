-TEST--
correctly handles setting map root
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php

$sass = new Sass();
// test default from constructor
$sass->setComments(true);
$sass->setMapRoot(__DIR__.'/support/');
$sass->setMapPath('map_root.css.map');
$css = $sass->compileFile(__DIR__.'/support/test.scss');
echo $css[1];
?>
--EXPECT--
{
    "version": 3,
    "file": "tests/support/test.css",
    "sources": [
            "tests/support/test.scss"
    ],
    "sourcesContent": [
            "@import url(../blahblah/blah.blah);\n\ndiv {\n  blah: \"hello #{2+2} world #{unit(23px)} #{'bloo'} blah\";\n}\n\n$foo: iphone;\n\ndiv {\n    blah: \"foo #{$foo$
    ],
    "names": [],
    "mappings": "AAAA,OAAO,CAAC,0BAAI;;AAEZ,AAAA,GAAG,CAAC;EACF,IAAI,EAAE,4BAAiD,GACxD;;;AAID,AAAA,GAAG,CAAC;EACA,IAAI,EAAE,YAAa,GACtB"
}
