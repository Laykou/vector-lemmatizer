<?php

	function lemma($lemma) {
		$url = 'http://text.fiit.stuba.sk:8080/lematizer/services/lemmatizer/lemmatize/full?tools=tvaroslovnik';
		$data = array('key1' => 'value1', 'key2' => 'value2');

		// use key 'http' even if you send the request to https://...
		$options = array(
		    'http' => array(
		        'header'  => "Content-Type: text/plain\r\n",
		        'method'  => 'POST',
		        'content' => $lemma,
		    ),
		);
		$context  = stream_context_create($options);
		return file_get_contents($url, false, $context);
	}

	while(fscanf(STDIN, "%s %s\n", $word, $lemma) == 2) {
		$l = lemma($word);
		fprintf(STDOUT, "%s\t%s\t%s\t%d\t%d\n", $word, $lemma, $l, ($lemma == $l ? 1 : 0), (strtolower($lemma) == strtolower($l) ? 1 : 0));
	}

?>