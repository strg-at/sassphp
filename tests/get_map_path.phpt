--TEST--
correctly handles getting source map path
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php

$sass = new Sass();
// test default from constructor
$sass->setMapPath('map_path.css.map');
echo $sass->getMapPath();

?>
--EXPECT--
map_path.css.map