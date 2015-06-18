<div class="row">
    <div class="col-lg-12 text-center">
        <h1>Vector lemmatizer</h1>
        <div class="row">
            <div class="col-md-6">
                <form id="input" role="form" class="form-horizontal" method="POST">
                    <div class="form-group">
                        <div class="col-md-12">
                            <textarea name="input" placeholder="Enter words to be lemmatized each on new line." class="form-control"></textarea>
                        </div>
                    </div>
                    <div class="form-group text-left">
                        <div class="col-md-12 text-right">
                            <button class="btn btn-link btn-sm">Submit</button>
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="choosing_method" class="col-md-4 control-label">Choosing method</label>
                        <div class="col-md-8">
                            <select class="form-control" name="choosing_method">
                                <option value="0">R0: Closest words</option>
                                <option value="1" selected="selected">R1: Suffix only</option>
                            </select>
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="iterations" class="col-md-4 control-label">Iterations</label>
                        <div class="col-md-8">
                            <input class="form-control" name="iterations" value="3">
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="word_analogy" class="col-md-4 control-label">Words to analyze</label>
                        <div class="col-md-8">
                            <input class="form-control" name="word_analogy" value="40">
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="weighting" class="col-md-4 control-label">Weighting</label>
                        <div class="col-md-8">
                            <select class="form-control" name="weighting">
                                <option value="0">W0: Common prefix</option>
                                <option value="1">W1: Jaro-Winkler distance</option>
                                <option value="2">W2: Levenshtein distance</option>
                                <option value="3">W3: Cosine distance only</option>
                            </select>
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="show_outputs" class="col-md-4 control-label">Show outputs</label>
                        <div class="col-md-8">
                            <input class="form-control" name="show_outputs" value="1">
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="print_pairs" class="col-md-4 control-label">Print pairs</label>
                        <div class="col-md-8">
                            <select class="form-control" name="print_pairs">
                                <option value="1">Yes</option>
                                <option value="0" selected="selected">No</option>
                            </select>
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="quiet" class="col-md-4 control-label">Quiet</label>
                        <div class="col-md-8">
                            <select class="form-control" name="quiet">
                                <option value="1">Yes</option>
                                <option value="0">No</option>
                            </select>
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="separator" class="col-md-4 control-label">Separator</label>
                        <div class="col-md-8">
                            <select class="form-control" name="separator">
                                <option value="lines">Lines</option>
                                <option value="words">Words</option>
                            </select>
                        </div>
                    </div>
                    <div class="form-group">
                        <div class="col-md-12">
                            <button class="btn btn-success btn-block btn-large">Submit</button>
                        </div>
                    </div>
                </form>
            </div>
            <div class="col-md-6">
                <div role="tabpanel">
                    <!-- Nav tabs -->
                    <ul class="nav nav-tabs" role="tablist">
                        <li role="presentation" class="active"><a href="#raw" aria-controls="raw" role="tab" data-toggle="tab">RAW</a></li>
                        <li role="presentation"><a href="#table" aria-controls="table" role="tab" data-toggle="tab">Table</a></li>
                    </ul>

                    <!-- Tab panes -->
                    <div class="tab-content">
                        <div role="tabpanel" class="tab-pane active" id="raw">
                            <form role="form" class="form-horizontal">
                                <div class="form-group">
                                    <div class="col-md-12">
                                        <textarea id="output" class="form-control"></textarea>
                                    </div>
                                </div>
                            </form>
                        </div>
                        <div role="tabpanel" class="tab-pane" id="table">
                            <table class="table table-bordered" id="output-table">
                            </table>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</div>
<!-- /.row -->