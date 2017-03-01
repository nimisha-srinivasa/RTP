package org.apache.lucene.demo;

/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Date;
//my
import java.util.Scanner;
//end

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.FilterIndexReader;
import org.apache.lucene.index.IndexReader;
//my
import org.apache.lucene.index.Term;
//end
import org.apache.lucene.queryParser.QueryParser;
import org.apache.lucene.search.Collector;
import org.apache.lucene.search.IndexSearcher;
//my
import org.apache.lucene.search.PhraseQuery;
import org.apache.lucene.search.UserInfo;
//end
import org.apache.lucene.search.Query;
import org.apache.lucene.search.ScoreDoc;
import org.apache.lucene.search.Scorer;
import org.apache.lucene.search.Searcher;
import org.apache.lucene.search.TopScoreDocCollector;
import org.apache.lucene.store.FSDirectory;
import org.apache.lucene.util.Version;

/** Simple command-line based search demo. */
public class SearchFiles {

  /** Use the norms from one field for all fields.  Norms are read into memory,
   * using a byte of memory per document per searched field.  This can cause
   * search of large collections with a large number of fields to run out of
   * memory.  If all of the fields contain only a single token, then the norms
   * are all identical, then single norm vector may be shared. */
  private static class OneNormsReader extends FilterIndexReader {
    private String field;

    public OneNormsReader(IndexReader in, String field) {
      super(in);
      this.field = field;
    }

    @Override
    public byte[] norms(String field) throws IOException {
      return in.norms(this.field);
    }
  }

  private SearchFiles() {}

  /** Simple command-line based search demo. */
  public static void main(String[] args) throws Exception {
    String usage =
      "Usage:\tjava org.apache.lucene.demo.SearchFiles [-index dir] [-field f] [-repeat n] [-queries file] [-raw] [-norms field] [-paging hitsPerPage]";
    usage += "\n\tSpecify 'false' for hitsPerPage to use streaming instead of paging search.";
    if (args.length > 0 && ("-h".equals(args[0]) || "-help".equals(args[0]))) {
      System.out.println(usage);
      System.exit(0);
    }

    String index = "index";
    String field = "contents";
    String queries = null;
    int repeat = 0;
    boolean raw = false;
    String normsField = null;
    boolean paging = true;
    int hitsPerPage = 10;
    
    for (int i = 0; i < args.length; i++) {
      if ("-index".equals(args[i])) {
        index = args[i+1];
        i++;
      } else if ("-field".equals(args[i])) {
        field = args[i+1];
        i++;
      } else if ("-queries".equals(args[i])) {
        queries = args[i+1];
        i++;
      } else if ("-repeat".equals(args[i])) {
        repeat = Integer.parseInt(args[i+1]);
        i++;
      } else if ("-raw".equals(args[i])) {
        raw = true;
      } else if ("-norms".equals(args[i])) {
        normsField = args[i+1];
        i++;
      } else if ("-paging".equals(args[i])) {
        if (args[i+1].equals("false")) {
          paging = false;
        } else {
          hitsPerPage = Integer.parseInt(args[i+1]);
          if (hitsPerPage == 0) {
            paging = false;
          }
        }
        i++;
      }
    }
    Date start, end;
    start = new Date();
    IndexReader reader = IndexReader.open(FSDirectory.open(new File(index)), true); // only searching, so read-only=true
    end = new Date();
	System.out.println("Read Indexing Time: "+(end.getTime()-start.getTime())+"ms");

    if (normsField != null)
      reader = new OneNormsReader(reader, normsField);

    Searcher searcher = new IndexSearcher(reader);
    Analyzer analyzer = new StandardAnalyzer(Version.LUCENE_CURRENT);

    BufferedReader in = null;
    if (queries != null) {
      in = new BufferedReader(new FileReader(queries));
    } else {
      in = new BufferedReader(new InputStreamReader(System.in, "UTF-8"));
    }
    QueryParser parser = new QueryParser(Version.LUCENE_CURRENT, field, analyzer);

    String line = args[0];
    for (int i = 1; i < args.length; i++)
	  line += " "+args[i];

    Date my_start = new Date();
    line = line.trim();
  
    Query query = parser.parse(line);
    System.out.println("Searching for: " + query.toString(field));

    //my
    PhraseQuery pq = new PhraseQuery();
    String querystring = query.toString(field);
  
    String delimiter = "[ ]+";
    String[] querytokens = querystring.split(delimiter);
  
    for (int i=0; i<querytokens.length; i++)
    {
	  Term queryterm = new Term("contents", querytokens[i]);
	  pq.add(queryterm);
    }
    int mySlotNumber = 10000;
    UserInfo.slot = mySlotNumber;
    pq.setSlop(mySlotNumber);
    if (mySlotNumber>0)
	  query = pq;
    //end
  
    if (repeat > 0) {                           // repeat & time as benchmark
    	start = new Date();
    	for (int i = 0; i < repeat; i++) {
    		searcher.search(query, null, 100);
    	}
    	end = new Date();
    	System.out.println("Time: "+(end.getTime()-start.getTime())+"ms");
    }

    if (paging) {
    	doPagingSearch(in, searcher, query, hitsPerPage, raw, queries == null);
    } else {
    	doStreamingSearch(searcher, query);
    }
    
    reader.close();
    Date my_end = new Date();
    System.out.println("Time: "+(my_end.getTime()-my_start.getTime())+"ms");
  }
  
  /**
   * This method uses a custom HitCollector implementation which simply prints out
   * the docId and score of every matching document. 
   * 
   *  This simulates the streaming search use case, where all hits are supposed to
   *  be processed, regardless of their relevance.
   */
  public static void doStreamingSearch(final Searcher searcher, Query query) throws IOException {
    Collector streamingHitCollector = new Collector() {
      private Scorer scorer;
      private int docBase;
      
      // simply print docId and score of every matching document
      @Override
      public void collect(int doc) throws IOException {
        System.out.println("doc=" + doc + docBase + " score=" + scorer.score());
      }

      @Override
      public boolean acceptsDocsOutOfOrder() {
        return true;
      }

      @Override
      public void setNextReader(IndexReader reader, int docBase)
          throws IOException {
        this.docBase = docBase;
      }

      @Override
      public void setScorer(Scorer scorer) throws IOException {
        this.scorer = scorer;
      }
      
    };
    
    searcher.search(query, streamingHitCollector);
  }

  /**
   * This demonstrates a typical paging search scenario, where the search engine presents 
   * pages of size n to the user. The user can then go to the next page if interested in
   * the next hits.
   * 
   * When the query is executed for the first time, then only enough results are collected
   * to fill 5 result pages. If the user wants to page beyond this limit, then the query
   * is executed another time and all hits are collected.
   * 
   */
  public static void doPagingSearch(BufferedReader in, Searcher searcher, Query query, 
                                     int hitsPerPage, boolean raw, boolean interactive) throws IOException {
 
    // Collect enough docs to show 5 pages
    TopScoreDocCollector collector = TopScoreDocCollector.create(
        5 * hitsPerPage, false);
    searcher.search(query, collector);
    ScoreDoc[] hits = collector.topDocs().scoreDocs;
  }
}
