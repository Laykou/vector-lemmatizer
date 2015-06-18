extern char *o();
extern unordered_map<wstring, int> wp;

int word_analogy(wchar_t *vocab, long long words, long long size, long long cn, wchar_t **st, float M[], float bestd[], wchar_t bestw[N][max_size])
{
	long long a, b = 0, c, d, bi[100];
	float dist, len;
	float vec[size];

	for (a = 0; a < cn; ++a)
	{
		try {
			bi[a] = (long long)wp.at(wstring(st[a]));
			++b;
			if(!quiet) {
				sprintf(o(), "Word: %ls  Position in vocabulary: %lld\n", st[a], bi[a]);
			}
		} catch (const out_of_range& r) {
			if(!quiet) {
				sprintf(o(), "Out of dictionary word: %ls!\n", st[a]);
			}
			return 0;
		}
	}

	if (b == 0)
		return 0;

	for (a = 0; a < size; ++a)
		vec[a] = M[a + bi[1] * size] - M[a + bi[0] * size] + M[a + bi[2] * size];
	len = 0;
	for (a = 0; a < size; ++a)
		len += vec[a] * vec[a];
	len = sqrt(len);
	for (a = 0; a < size; ++a)
		vec[a] /= len;
	for (a = 0; a < word_analogy_iterations; ++a)
		bestd[a] = 0;
	for (a = 0; a < word_analogy_iterations; ++a)
		bestw[a][0] = 0;
	for (c = 0; c < words; c++)
	{
		if (c == bi[0]) continue;
		if (c == bi[1]) continue;
		if (c == bi[2]) continue;
		a = 0;
		for (b = 0; b < cn; ++b)
		{
			if (bi[b] == c)
				a = 1;
		}
		if (a == 1)
			continue;
		dist = 0;
		for (a = 0; a < size; ++a)
			dist += vec[a] * M[a + c * size];
		for (a = 0; a < word_analogy_iterations; ++a)
		{
			if (dist > bestd[a])
			{
				for (d = word_analogy_iterations - 1; d > a; --d)
				{
					bestd[d] = bestd[d - 1];
					wcscpy(bestw[d], bestw[d - 1]);
				}
				bestd[a] = dist;
				wcscpy(bestw[a], &vocab[c * max_w]);
				break;
			}
		}
	}

	return 1;
}