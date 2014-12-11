<?php

namespace core\php;

error_reporting(E_ALL);

require_once 'ZeonClient.php';

$x = $_REQUEST['x'];
$y = $_REQUEST['y'];

$error = "";

function printResult($ret) {
  $obj = array();
  foreach ($ret as $d) {
    $obj[] = array('x' => $d->point->xCord, 'y' => $d->point->yCord);
  }
  echo json_encode($obj);
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
  // Pick any node, let server redirect
  try {
    add_point('localhost', 8000, $x, $y, $_POST['id']);
  } catch (\core\ZeonException $ze) {
    if ($ze->what != \core\ErrorCode::SERVER_REDIRECT) {
      $error = \core\ErrorCode::__names[$ze->what];
    } else {
      $nodes = $ze->nodes;
      // Contact other nodes
      foreach($nodes as $node) {
        try {
          add_point($node->ip, $node->clientPort, $x, $y, $_POST['id']);
          break;
        } catch (\core\ZeonException $ze) {
          $error = \core\ErrorCode::__names[$ze->what];
        }
      } 
    }
  }
  if ($error === "") {
    http_response_code(200);
  } else {
    http_response_code(400);
  }
} else {
  try {
    $ret = get_nearest('localhost', 8000, $x, $y);
    printResult($ret);
  } catch (\core\ZeonException $ze) {
    if ($ze->what != \core\ErrorCode::SERVER_REDIRECT) {
      $error = \core\ErrorCode::__names[$ze->what];
    } else {
      $nodes = $ze->nodes;
      // Contact other nodes
      foreach($nodes as $node) {
        try {
          $ret = get_nearest($node->ip, $node->clientPort, $x, $y);
          printResult($ret);
          break;
        } catch (\core\ZeonException $ze) {
          $error = \core\ErrorCode::__names[$ze->what];
        }
      } 
    }
  }
  if ($error === "") {
    http_response_code(200);
  } else {
    http_response_code(400);
  }
}


?>