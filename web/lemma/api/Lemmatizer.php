<?php

/**
  * Wrapper for the socket connection to the lemmatizer
  */
class Lemmatizer
{
    protected $_options = [];
    protected $_connection = null;
    protected $_host = null;
    protected $_port = null;

    public function __construct($host = "127.0.0.1", $port = 4332)
    {
        $this->_host = $host;
        $this->_port = $port;
    }

    protected function _reportError($msg)
    {
        $errorcode = socket_last_error();
        $errormsg = socket_strerror($errorcode);
        throw new RuntimeException("$msg [$errorcode]: $errormsg");
    }

    protected function _establish()
    {
        $connection = @socket_create(AF_INET, SOCK_STREAM, 0);
        if(!$connection) {
            $this->_reportError("Could not create socket");
        }
        
        if(!@socket_connect($connection, $this->_host, $this->_port)) {
            $this->_reportError("Could not connect");
        }

        return $connection;
    }

    public function set($name, $value = null)
    {
        if ($value === null && isset($this->_options[$name])) {
            return $this->_options[$name];
        }
        $this->_options[$name] = $value;
        return $value;
    }

    public function lemmatize($word, $options = [])
    {
        $opts = $this->_options + $options;
        $message = [];
        foreach($opts as $key => $value) {
            $message[] = "-" . $key;
            if ($value !== true) {
                $message[] = $value;
            }
        }
        $message[] = $word;
        return $this->send(join(' ', $message));
    }
     
    public function send($message)
    {
        $socket = $this->_establish();

        if(socket_send($socket, $message, strlen($message), 0) === false) {
            $this->_reportError("Could not send data");
        }
        
        if(socket_recv($socket, $buf, MESSAGE, MSG_WAITALL) === false) {
            $this->_reportError("Could not receive data");
        }

        socket_close($socket);

        return $buf;
    }
}