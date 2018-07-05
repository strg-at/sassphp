--TEST--
correctly handles getting source map root
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php

$sass = new Sass();
// test default from constructor
$sass->setMapRoot('/sources/');
echo $sass->getMapRoot();

?>
--EXPECT--
/sources/
