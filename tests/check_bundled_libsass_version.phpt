--TEST--
check bundled libsass version
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php

echo version_compare(Sass::getLibraryVersion(), '3.5.2')

?>
--EXPECT--
0
