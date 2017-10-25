<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Form for uploading a custom XSLT transform.
 */
function islandora_ead_admin_settings_form($form, &$form_state) {
  // Use the #managed_file FAPI element to upload an XSLT file.
  $form['islandora_ead_xslt_fid'] = array(
    '#title' => t('Custom XSLT'),
    '#type' => 'managed_file',
    '#description' => t('The uploaded XSLT will be used to transform the EAD into HTML.'),
    '#default_value' => variable_get('islandora_ead_xslt_fid', ''),
    '#upload_location' => 'public://islandora_ead/',
  );

  // Submit Button.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Verifies that the user supplied an XSLT with the form..
 */
function islandora_ead_admin_settings_form_validate($form, &$form_state) {
  if (!isset($form_state['values']['islandora_ead_xslt_fid']) || !is_numeric($form_state['values']['islandora_ead_xslt_fid'])) {
    form_set_error('islandora_ead_xslt_fid', t('Please select an XSLT to upload.'));
  }
}

/**
 * Form Builder; Display a form for uploading an XSLT.
 */
function islandora_ead_admin_settings_form_submit($form, &$form_state) {
  // When using the #managed_file form element the file is automatically
  // uploaded an saved to the {file} table. The value of the corresponding
  // form element is set to the {file}.fid of the new file.
  //
  // If fid is not 0 we have a valid file.
  if ($form_state['values']['islandora_ead_xslt_fid'] != 0) {
    // The new file's status is set to 0 or temporary and in order to ensure
    // that the file is not removed after 6 hours we need to change it's status
    // to 1. Save the ID of the uploaded image for later use.
    $file = file_load($form_state['values']['islandora_ead_xslt_fid']);
    $file->status = FILE_STATUS_PERMANENT;
    file_save($file);

    // When a module is managing a file, it must manage the usage count.
    // Here we increment the usage count with file_usage_add().
    file_usage_add($file, 'islandora_solution_pack_ead', 'ead_xslt', 1);

    // Save the fid of the file so that the module can reference it later.
    variable_set('islandora_ead_xslt_fid', $file->fid);
    drupal_set_message(t('The XSLT @xslt_name was uploaded and saved with an ID of @fid.',
      array(
        '@xslt_name' => $file->filename,
        '@fid' => $file->fid,
      )
    ));
  }
  // If the file was removed we need to remove the module's reference to the
  // removed file's fid, and remove the file.
  elseif ($form_state['values']['islandora_ead_xslt_fid'] == 0) {
    // Retrieve the old file's id.
    $fid = variable_get('islandora_ead_xslt_fid', FALSE);
    $file = $fid ? file_load($fid) : FALSE;
    if ($file) {
      // When a module is managing a file, it must manage the usage count.
      // Here we decrement the usage count with file_usage_delete().
      file_usage_delete($file, 'islandora_solution_pack_ead', 'ead_xslt', 1);

      // The file_delete() function takes a file object and checks to see if
      // the file is being used by any other modules. If it is the delete
      // operation is cancelled, otherwise the file is deleted.
      file_delete($file);
    }

    // Either way the module needs to update it's reference since even if the
    // file is in use by another module and not deleted we no longer want to
    // use it.
    variable_set('islandora_ead_xslt_fid', FALSE);
    drupal_set_message(t('The XSLT @xslt_name was removed.', array('@xslt_name' => $file->filename)));
  }
}
