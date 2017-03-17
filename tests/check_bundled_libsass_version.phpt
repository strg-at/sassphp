--TEST--
check bundled libsass version
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php

echo version_compare(Sass::getLibraryVersion(), '3.4.3')

?>
--EXPECT--
0
