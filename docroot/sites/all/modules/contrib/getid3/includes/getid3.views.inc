<?php
/**
 * @file
 * Provide views data for getid3.module.
 */

/**
 * Implements hook_views_data()
 */
function getid3_views_data() {
  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['getid3_meta']['table']['group']  = t('File');

  // For other base tables, explain how we join
  $data['getid3_meta']['table']['join'] = array(
    // this explains how the 'getid3_meta' table (named in the line above)
    // links toward the files table.
    'file_managed' => array(
      'left_table' => 'file_managed', // Because this is a direct link it could be left out.
      'left_field' => 'fid',
      'field' => 'fid',
     ),
  );

  // ----------------------------------------------------------------
  // getid3_meta table -- fields

  // width
  $data['getid3_meta']['width'] = array(
    'title' => t('Width'),
    'help' => t('Width of a video or image file in pixels.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // height
  $data['getid3_meta']['height'] = array(
    'title' => t('Height'),
    'help' => t('Height of a video or image file in pixels.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // duration
  $data['getid3_meta']['duration'] = array(
    'title' => t('Duration'),
    'help' => t('The duration of audio or video files, in seconds.'),
    'field' => array(
      'handler' => 'getid3_handler_field_duration',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // audio_format
  $data['getid3_meta']['audio_format'] = array(
    'title' => t('Audio format'),
    'help' => t('The audio format.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // audio_sample_rate
  $data['getid3_meta']['audio_sample_rate'] = array(
    'title' => t('Audio sample rate'),
    'help' => t('The sample rate of the audio.'),
    'field' => array(
      'handler' => 'getid3_handler_field_sample_rate',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // audio_channel_mode
  $data['getid3_meta']['audio_channel_mode'] = array(
    'title' => t('Audio channel mode'),
    'help' => t('The number of channels in the audio, by name: stereo or mono.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // audio_bitrate
  $data['getid3_meta']['audio_bitrate'] = array(
    'title' => t('Audio bitrate'),
    'help' => t('The audio bitrate.'),
    'field' => array(
      'handler' => 'getid3_handler_field_bitrate',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // audio_bitrate_mode
  $data['getid3_meta']['audio_bitrate_mode'] = array(
    'title' => t('Audio bitrate mode'),
    'help' => t('The audio bitrate mode: cbr, vbr, abr.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  //// Tags.
  //$data['getid3_meta']['tags'] = array(
  //  'title' => t('ID3 tags'),
  //  'help' => t('ID3 tags include embedded information such as artist, album, year, genre and other informaiton.'),
  //  'field' => array(
  //    'handler' => 'getid3_handler_field_tags',
  //    'click sortable' => FALSE,
  //  ),
  //);

  return $data;
}
