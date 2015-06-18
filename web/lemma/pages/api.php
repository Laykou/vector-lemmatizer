<div class="text-left">
	<h1>API</h1>
	<p class="lead">
		Text lemmatization utilizing vector models. Based on the <a href="https://code.google.com/p/word2vec/">word2vec tool</a>.
	</p>
	<h2>URL</h2>
	<pre>http://localhost/lemma/api/</pre>
	<h2>Type</h2>
	<pre>POST</pre>
	<h2>Options</h2>
	<p>
		You can pass options as query parameters, form data or JSON content. You can use the same options as when running the application
		in console mode. Default values may change according to run attributes of the lemmatizer.
	</p>
	<table class="table table-bordered">
		<thead>
			<tr>
				<th>Name</th>
				<th>Description</th>
				<th>Values</th>
				<th>Default value</th>
			</tr>
		</thead>
		<tbody>
			<tr>
				<td>choosing_method</td>
				<td>Modifies the method of choosing the reference words from the reference lexicon.</td>
				<td>
					<ul class="list-unstyled">
						<li>0 - Closer words to the input word will be evaluated first (if exist in lexicon). See the closestCount variable to set how many closest words will be retrieved for input word.
						<li>1 - Words from reference lexion are chosen by the longest common suffix.	
					</ul>
				</td>
				<td>1</td>
			</tr>
			<tr>
				<td>closest_count</td>
				<td>If the choosing method is set to 0 (closer words), this number denotes how many words will be fetched from the distance() method when retrieving the list of closest words for given input.</td>
				<td>integer</td>
				<td>40</td>
			</tr>
			<tr>
				<td>show_outputs</td>
				<td>Denotes the number of top words that will be printed. The results are sorted by their weights and only the first result should be the correct lemma. However for testing purposes is good to see other positions.</td>
				<td>integer</td>
				<td>1</td>
			</tr>
			<tr>
				<td>weighting</td>
				<td>Each unique word from the word_analogy() method is weighted.</td>
				<td>
					<ul class="list-unstyled">
						<li> 0 - Relative common prefix - prefix length of the input word and the lemma candidate (normalized)
 						<li> 1 - Jaro-winkler distance
 						<li> 2 - Levenshtein distance
 						<li> 3 - Cosine distance only (no weighting performed, only the distance from word_analogy() method is used)
 					</ul>
				</td>
				<td>0</td>
			</tr>
			<tr>
				<td>iterations</td>
				<td>Number of iterations - number of distinct reference pairs that will be chosen and evaluated by the word_analogy() method.</td>
				<td>integer</td>
				<td>3</td>
			</tr>
			<tr>
				<td>print_pairs</td>
				<td>Boolean indicates whether to print the reference pairs in the output that were used in word_analogy() method. Number of this pairs is determined by the "match_iterations" variable (-iterations argument).<br>
 					<br>
 					Standard behavior is to print "[input] [results]", e.g. for input "autom auto 10": "autom auto 10 auto 0,974 motorka 0,876".<br>
 					Printing the pairs will output "[input] [pairs] [result]", e.g. for 2 iterations: "autom auto 10 dubom dub chlapom chlap auto 0,974 motorka 0,876".</td>
				<td>1/0</td>
				<td>0</td>
			</tr>
			<tr>
				<td>quiet</td>
				<td>If false each input line will have only one-line output. The input will be bypassed to the output followed by the results. If true (1) more debug information is being printed as the algorithm processes the input.</td>
				<td>1/0</td>
				<td>0</td>
			</tr>
			<tr>
				<td>type</td>
				<td>Response type</td>
				<td>text/json</td>
				<td>json</td>
			</tr>
			<tr>
				<td>separator</td>
				<td>You can pass multiple inputs at once. They will be separated either by the line ending or by any whitespace.</td>
				<td>
					<ul class="list-unstyled">
						<li> lines - Each input is on a new line. Only the first word is lemmatized, rest is forwarded to the output.
						<li> words - Each word is lemmatized
 					</ul>
				</td>
				<td>lines</td>
			</tr>
			<tr>
				<td><b>input</b></td>
				<td>The input to be lemmatized. See the <i>separator</i> for multicase input.
				<td>string</td>
				<td></td>
			</tr>
		</tbody>
	</table>
	<p>
		Additionaly you can pass the attributes in the URL query and input as the raw data content.
	</p>
	<h2>Response</h2>
	<p>
		A <b>text</b> response the output is the same as to the stdout in lemmatizer. If <i>quiet</i> is turned on each input has one output line with multiple words separated by tabulator. The format of output is "[input] [results]".
	</p>
	<pre>http://localhost/lemma/api/?type=text&show_outputs=3

autami	auto	0,141678	autá	0,105800	autiak	0,097429
motorky	motorka	0,188297	motocykel	0,187217	motokáry	0,177604
plotoch	plot	0,103084	peň	0,060099	platan	0,054166</pre>
	<p>
		<b>JSON</b> response is similiar to the <i>text</i> response. It contains two keys: <b>status</b> true/false and <b>results</b>. Results is an array of outputs as on stdin. Each input has separated item in the results array. Each item is a tab-separated string as in <i>text</i> response.
	</p>
	<pre>{
    "status": true,
    "results": [
        "autami\tauto\t0,141678",
        "motorky\tmotorka\t0,188297",
        "plotoch\tplot\t0,103084"
    ]
}</pre>
	<p>
		If the <i>status</i> is false, the <b>message</b> key contains the error details.
	</p>
	<h2>Examples</h2>
	<pre>URL: http://localhost/lemma/api/
Data: {
	"iterations": 2,
	"print_pairs": 1,
	"weighting": 2,
	"word_analogy": 30,
	"input": "áut auto X1 A\nmotorky X2 B\nzime X3 C"
}

Response:
{
    "status": true,
    "results": [
        "áut\tauto\tX1\tA\tdievčat\tdievča\tmláďat\tmláďa\tauto\t0,365384",
        "motorky\tX2\tB\truky\truka\tduby\tdub\tmotorka\t0,282446",
        "zimám\tX3\tC\tknihám\tkniha\trukám\truka\tzima\t0,134633"
    ]
}</pre>
	<pre>URL: http://localhost/lemma/api/?separator=words&type=text
Data: kľúčiku dome kabelky

Response:
kľúčiku	kľúčik	0,151943
dome	dom	0,294188
kabelky	kabelka	0,338623</pre>
</div>