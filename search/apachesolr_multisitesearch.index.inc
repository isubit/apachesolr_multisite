<?php

function apachesolr_multisitesearch_refresh_metadata() {
  // Fetch meta data 1x per 5 minutes max.
  $last_fetch = variable_get('apachesolr_multisitesearch_last_metadata_fetch', 0);
  $time = REQUEST_TIME;
  if ($time - $last_fetch > variable_get('apachesolr_multisitesearch_metadata_fetch_interval', 300)) {
    apachesolr_multisitesearch_retrieve_metadata();
  }
  $env_id = apachesolr_default_environment();
  apachesolr_set_last_index_updated($env_id, $time);
}

function apachesolr_multisitesearch_retrieve_metadata() {
  try {
    // Get the $solr object
    $solr = apachesolr_get_solr();
    $params['defType'] = 'lucene';
    $params['fl'] = '*';
    $response = $solr->search('entity_type:multisite.meta', $params);
    $data = array();
    foreach ($response->response->docs as $doc) {
      // Convert doc into a simple array.
      if (isset($doc->hash)) {
        foreach ($doc as $k => $v) {
          $data[$doc->hash][$k] = $v;
        }
        if (empty($data[$doc->hash]['sm_multisite_meta_taxonomy'])) {
          $data[$doc->hash]['sm_multisite_meta_taxonomy'] = array();
        }
        if (empty($data[$doc->hash]['sm_multisite_meta_bundles'])) {
          $data[$doc->hash]['sm_multisite_meta_bundles'] = array();
        }
      }
    }
    watchdog('Apache Solr Multisite Search', 'Fetched site meta data');
    variable_set('apachesolr_multisitesearch_metadata', $data);
  }
  catch (Exception $e) {
    watchdog('Apache Solr Multisite Search', 'Failed to fetch meta data <br /> !message',
      array('!message' => nl2br(strip_tags($e->getMessage()))), WATCHDOG_ERROR);
  }
}

function apachesolr_multisitesearch_get_site_hashes() {
  try {
    // Get the $solr object
    $solr = apachesolr_get_solr();
    $params['defType'] = 'lucene';
    $params['fl'] = '*';
    $params['facet'] = 'true';
    $params['facet.field'][] = 'hash';
    $params['facet.mincount'] = 1;
    $params['facet.limit'] = '1000';
    $response = $solr->search('*:*', $params);
    $results = (array)$response->facet_counts->facet_fields->hash;
    return $results;
  }
  catch (Exception $e) {
    watchdog('Apache Solr Multisite Search', 'Failed to fetch hash facet count <br /> !message',
             array('!message' => nl2br(strip_tags($e->getMessage()))), WATCHDOG_ERROR);
    return array();
  }
}
