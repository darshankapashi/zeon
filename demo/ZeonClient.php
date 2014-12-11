<?php

namespace core\php;

$thrift_root = 'thrift-0.9.1/lib';
// Load up all the thrift stuff
require_once $thrift_root.'/Thrift/ClassLoader/ThriftClassLoader.php';

use Thrift\ClassLoader\ThriftClassLoader;

$GEN_DIR = '../gen-php';

$loader = new ThriftClassLoader();
$loader->registerNamespace('Thrift', $thrift_root);
$loader->registerDefinition('core', $GEN_DIR);
$loader->register();

use Thrift\Protocol\TBinaryProtocol;
use Thrift\Transport\TSocket;
use Thrift\Transport\THttpClient;
use Thrift\Transport\TBufferedTransport;
use Thrift\Exception\TException;

function add_point($ip, $port, $x, $y, $id) {
  $socket = new TSocket($ip, $port);
  $transport = new TBufferedTransport($socket, 1024, 1024);
  $protocol = new TBinaryProtocol($transport);
  $client = new \core\PointStoreClient($protocol);
  $transport->open();

  $point = new \core\Point();
  $point->xCord = $x;
  $point->yCord = $y;
  return $client->createData($id, $point, 0, "meh");
}

function get_nearest($ip, $port, $x, $y) {
  $socket = new TSocket($ip, $port);
  $transport = new TBufferedTransport($socket, 1024, 1024);
  $protocol = new TBinaryProtocol($transport);
  $client = new \core\PointStoreClient($protocol);
  $transport->open();

  $point = new \core\Point();
  $point->xCord = $x;
  $point->yCord = $y;
  return $client->getNearestKByPoint($point, 3);
}


?>