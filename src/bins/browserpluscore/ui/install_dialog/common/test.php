<?php
// Small frontend on index.html so we can develop with a faked window.BPDialog object
echo str_replace("service_install.js", "test_js.php", file_get_contents("index.html"));
?>