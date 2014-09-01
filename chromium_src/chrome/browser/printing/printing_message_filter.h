// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PRINTING_PRINTING_MESSAGE_FILTER_H_
#define CHROME_BROWSER_PRINTING_PRINTING_MESSAGE_FILTER_H_

#include <string>

#include "base/compiler_specific.h"
#include "content/public/browser/browser_message_filter.h"

#if defined(OS_WIN)
#include "base/memory/shared_memory.h"
#endif

struct PrintHostMsg_ScriptedPrint_Params;

namespace base {
class DictionaryValue;
class FilePath;
}

namespace content {
class WebContents;
}

namespace printing {
class PrinterQuery;
class PrintJobManager;
class PrintQueriesQueue;
}

// This class filters out incoming printing related IPC messages for the
// renderer process on the IPC thread.
class PrintingMessageFilter : public content::BrowserMessageFilter {
 public:
  explicit PrintingMessageFilter(int render_process_id);

  // content::BrowserMessageFilter methods.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  virtual ~PrintingMessageFilter();

#if defined(OS_WIN)
  // Used to pass resulting EMF from renderer to browser in printing.
  void OnDuplicateSection(base::SharedMemoryHandle renderer_handle,
                          base::SharedMemoryHandle* browser_handle);
#endif

  // Given a render_view_id get the corresponding WebContents.
  // Must be called on the UI thread.
  content::WebContents* GetWebContentsForRenderView(int render_view_id);

  // GetPrintSettingsForRenderView must be called via PostTask and
  // base::Bind.  Collapse the settings-specific params into a
  // struct to avoid running into issues with too many params
  // to base::Bind.
  struct GetPrintSettingsForRenderViewParams;

  // Retrieve print settings.  Uses |render_view_id| to get a parent
  // for any UI created if needed.
  void GetPrintSettingsForRenderView(
      int render_view_id,
      GetPrintSettingsForRenderViewParams params,
      const base::Closure& callback,
      scoped_refptr<printing::PrinterQuery> printer_query);

  void OnGetPrintSettingsFailed(
      const base::Closure& callback,
      scoped_refptr<printing::PrinterQuery> printer_query);

  // Get the default print setting.
  void OnGetDefaultPrintSettings(IPC::Message* reply_msg);
  void OnGetDefaultPrintSettingsReply(
      scoped_refptr<printing::PrinterQuery> printer_query,
      IPC::Message* reply_msg);

  // The renderer host have to show to the user the print dialog and returns
  // the selected print settings. The task is handled by the print worker
  // thread and the UI thread. The reply occurs on the IO thread.
  void OnScriptedPrint(const PrintHostMsg_ScriptedPrint_Params& params,
                       IPC::Message* reply_msg);
  void OnScriptedPrintReply(
      scoped_refptr<printing::PrinterQuery> printer_query,
      IPC::Message* reply_msg);

  const int render_process_id_;

  scoped_refptr<printing::PrintQueriesQueue> queue_;

  DISALLOW_COPY_AND_ASSIGN(PrintingMessageFilter);
};

#endif  // CHROME_BROWSER_PRINTING_PRINTING_MESSAGE_FILTER_H_
