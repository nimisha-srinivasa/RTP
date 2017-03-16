Representative-guided Two-Phase Search (RTP) Graphical User Interface (GUI)
=======================================
How to run the GUI
------------

1) make sure apache HTTP server is running. [http://httpd.apache.org/docs/2.4/install.html]

2) make sure /var/www/html contains search.html and css/ folder (for Unix environment).

3) make sure /var/www/cgi-bin contains the search.sh and run_single_query.sh files (for Unix environment).

4) run make on the root RTP directory

5) run ./index.sh &lt;xml_data_file&gt; &lt;representative_choice&gt; to generate the index files

6) copy the target directory to /var/www/cgi-bin folder

7) navigate to /var/www/cgi-bin directory using command-line and type the following command:

`chmod -R 777 target`

8) go to 127.0.0.1/search.html on your browser to run the gui

How to use the GUI
------------

1)Type in the conjunctive query to be searched in the Search Box. 

2)Click on the drop-down and input the value of the top_k parameter if desired

3) click on the search icon to run the search.