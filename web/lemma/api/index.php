<?php
    define('MESSAGE', 100000);

    // List of parameters that are API only and will not be passed to the vector lemmatizer as options
    $apiParams = ['input', 'separator', 'type'];

    require_once 'Lemmatizer.php';

    $l = new Lemmatizer();

    $defaultOptions = [
        'quiet' => 1,
        'separator' => 'lines', // lines or words
        'type' => 'json'
    ];

    $options = array_merge($defaultOptions, $_GET, $_POST);

    // Parse the input
    if (isset($_POST['input'])) {
        $input = $_POST['input'];
    } else {
        $input = file_get_contents("php://input");
        $json = json_decode($input, true);
        if ($json !== null) {
            $options = array_merge($options, $json);
            $input = isset($json['input']) ? $json['input'] : "";
        }
    }

    // Set the lematizer options as passed from the request
    foreach ($options as $key => $value) {
        if (!in_array($key, $apiParams)) {
            $l->set($key, $value);
        }
    }

    if ($options['separator'] === 'words') {
        $inputCases = preg_split('/\s+/', $input);
    } else {
        $inputCases = explode("\n", $input);
    }

    $inputCases = array_map('trim', $inputCases);
    $inputCases = array_filter($inputCases);

    $results = [];

    try {
        foreach ($inputCases as $case) {
            $results[] = trim($l->lemmatize("$case"));
        }

        $response = [
            'status' => true,
            'results' => $results
        ];
    } catch(Exception $e) {
        $response = [
            'status' => false,
            'message' => $e->getMessage()
        ];
    }

    // PRINT THE RESPONSE

    if ($response['status']) {
        http_response_code(200);
    } else {
        http_response_code(500); // Internall error
    }

    if ($options['type'] === 'text') {
        header('Content-Type: plain/text');
        if (isset($response['message'])) {
            echo $response['message'];
        } else {
            echo join("\n", $response['results']);
        }
    }  else {
        header('Content-Type: application/json; charset=utf-8');
        echo json_encode($response);
    }