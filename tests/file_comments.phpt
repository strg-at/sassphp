--TEST--
compiles correct Sass file and provides source comments
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php

$sass = new Sass();
$sass->setComments(true);
$css = $sass->compileFile(__DIR__.'/support/test.scss');
echo $css;


?>
--EXPECTF--
@import url(../blahblah/blah.blah);
/* line 3, %s/support/test.scss */
div {
  blah: "hello 4 world px bloo blah"; }

/* line 9, %s/support/test.scss */
div {
  blah: "foo iphone"; }

