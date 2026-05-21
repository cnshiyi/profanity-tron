using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace ProfanityTronStudio
{
    internal static class UiText
    {
        public static string T(string hex)
        {
            var chars = hex.Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries)
                .Select(part => (char)Convert.ToInt32(part, 16))
                .ToArray();
            return new string(chars);
        }
    }

    internal static class Program
    {
        [STAThread]
        private static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.ThreadException += (sender, args) => ShowError(args.Exception);
            AppDomain.CurrentDomain.UnhandledException += (sender, args) => ShowError(args.ExceptionObject as Exception);
            Application.Run(new MainForm());
        }

        private static void ShowError(Exception error)
        {
            if (error == null) return;
            MessageBox.Show(error.ToString(), UiText.T("547D 4EE4 9519 8BEF"), MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }

    internal sealed class MainForm : Form
    {
        private readonly string rootDir;
        private readonly string exePath;
        private readonly string runtimeDir;
        private readonly string runOutputDir;
        private readonly string defaultOutputPath;
        private readonly string defaultTargetsPath;

        private readonly Button startButton;
        private readonly Button stopButton;
        private readonly Button pickOutputButton;
        private readonly Button openOutputButton;
        private readonly Button copyButton;
        private readonly Button clearButton;
        private readonly Button saveTargetButton;
        private readonly Button exitButton;

        private readonly TextBox targetEditor;
        private readonly TextBox outputBox;
        private readonly NumericUpDown prefixBox;
        private readonly NumericUpDown suffixBox;
        private readonly NumericUpDown countBox;
        private readonly Label statusValue;
        private readonly Label pidValue;
        private readonly Label hitCountValue;
        private readonly Label brandLabel;
        private readonly SplitContainer mainSplit;
        private readonly DataGridView resultGrid;
        private readonly ContextMenuStrip resultMenu;
        private readonly TextBox logBox;
        private readonly Timer pollTimer;
        private readonly HashSet<string> seenHits = new HashSet<string>(StringComparer.Ordinal);
        private readonly HashSet<string> importedRunHits = new HashSet<string>(StringComparer.Ordinal);

        private Process runningProcess;
        private int runningProcessId;
        private IntPtr runningJobHandle;
        private string lastOutputPath;
        private int hitCount;
        private bool shuttingDown;
        private bool splitLayoutReady;

        public MainForm()
        {
            rootDir = AppDomain.CurrentDomain.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            exePath = Path.Combine(rootDir, "profanity.x64.exe");
            runtimeDir = Path.Combine(rootDir, "runtime");
            runOutputDir = Path.Combine(runtimeDir, "runs");
            defaultOutputPath = Path.Combine(runtimeDir, "hits_all.txt");
            defaultTargetsPath = Path.Combine(runtimeDir, "targets.txt");
            lastOutputPath = defaultOutputPath;
            Directory.CreateDirectory(runtimeDir);
            Directory.CreateDirectory(runOutputDir);

            Text = UiText.T("6CE2 573A 9753 53F7 751F 6210 5DE5 5177");
            Width = 1495;
            Height = 850;
            MinimumSize = new Size(1280, 720);
            StartPosition = FormStartPosition.CenterScreen;
            BackColor = Color.FromArgb(238, 238, 238);
            Font = UiFont(9f);

            var topPanel = new Panel { Dock = DockStyle.Top, Height = 128, BackColor = Color.FromArgb(238, 238, 238) };
            Controls.Add(topPanel);

            startButton = AddButton(topPanel, UiText.T("5F00 59CB 751F 6210"), 22, 18, 100);
            stopButton = AddButton(topPanel, UiText.T("505C 6B62 4EFB 52A1"), 142, 18, 100);
            pickOutputButton = AddButton(topPanel, UiText.T("9009 62E9 8F93 51FA"), 262, 18, 100);
            copyButton = AddButton(topPanel, UiText.T("590D 5236 7ED3 679C"), 382, 18, 100);
            clearButton = AddButton(topPanel, UiText.T("6E05 7A7A 5217 8868"), 502, 18, 100);
            saveTargetButton = AddButton(topPanel, UiText.T("4FDD 5B58 76EE 6807"), 622, 18, 100);
            exitButton = AddButton(topPanel, UiText.T("9000 51FA 7A0B 5E8F"), 742, 18, 100);
            stopButton.Enabled = false;

            AddLabel(topPanel, UiText.T("524D 7F00"), 22, 74, 36);
            prefixBox = AddNumberBox(topPanel, 60, 70, 58, 0, 10, 0);
            AddLabel(topPanel, UiText.T("540E 7F00"), 130, 74, 36);
            suffixBox = AddNumberBox(topPanel, 168, 70, 58, 0, 12, 5);
            AddLabel(topPanel, UiText.T("6570 91CF"), 240, 74, 36);
            countBox = AddNumberBox(topPanel, 278, 70, 70, 1, 1000000, 10);

            AddLabel(topPanel, UiText.T("8F93 51FA 6587 4EF6"), 22, 106, 58);
            outputBox = AddTextBox(topPanel, 82, 102, 330, defaultOutputPath);
            openOutputButton = AddButton(topPanel, UiText.T("6253 5F00"), 422, 96, 72);
            AddLabel(topPanel, UiText.T("72B6 6001"), 520, 106, 40);
            statusValue = AddValueLabel(topPanel, UiText.T("7A7A 95F2"), 563, 106, 72);
            AddLabel(topPanel, "PID", 645, 106, 30);
            pidValue = AddValueLabel(topPanel, "-", 677, 106, 80);
            AddLabel(topPanel, UiText.T("547D 4E2D"), 770, 106, 40);
            hitCountValue = AddValueLabel(topPanel, "0", 813, 106, 80);
            hitCountValue.ForeColor = Color.Blue;
            brandLabel = new Label
            {
                Text = "@shiyi",
                Anchor = AnchorStyles.Top | AnchorStyles.Right,
                TextAlign = ContentAlignment.MiddleRight,
                Location = new Point(1240, 30),
                Size = new Size(190, 58),
                Font = UiFont(18f, FontStyle.Regular),
                ForeColor = Color.FromArgb(45, 45, 45)
            };
            topPanel.Controls.Add(brandLabel);

            var bottomPanel = new Panel { Dock = DockStyle.Bottom, Height = 240, Padding = new Padding(8, 6, 8, 10), BackColor = Color.FromArgb(238, 238, 238) };
            Controls.Add(bottomPanel);
            var logTitle = new Label { Text = UiText.T("8F93 51FA 65E5 5FD7"), Dock = DockStyle.Top, Height = 24, Font = UiFont(9f, FontStyle.Bold) };
            bottomPanel.Controls.Add(logTitle);
            logBox = new TextBox { Dock = DockStyle.Fill, Multiline = true, ScrollBars = ScrollBars.Both, WordWrap = false, ReadOnly = true, Font = new Font("Consolas", 9f), BackColor = Color.White };
            bottomPanel.Controls.Add(logBox);
            logBox.BringToFront();

            mainSplit = new SplitContainer { Dock = DockStyle.Fill, BackColor = Color.FromArgb(238, 238, 238), Padding = new Padding(4, 0, 4, 0) };
            Controls.Add(mainSplit);
            mainSplit.BringToFront();

            var targetGroup = new GroupBox { Text = UiText.T("76EE 6807 5730 5740 7F16 8F91"), Dock = DockStyle.Fill, Font = UiFont(9f, FontStyle.Bold) };
            mainSplit.Panel1.Controls.Add(targetGroup);
            var targetFooter = new Panel { Dock = DockStyle.Bottom, Height = 28, BackColor = Color.White };
            targetGroup.Controls.Add(targetFooter);
            var targetTip = new Label { Dock = DockStyle.Fill, Text = UiText.T("6BCF 884C 4E00 4E2A 76EE 6807 5730 5740 FF0C 4FDD 5B58 540E 5F00 59CB 751F 6210 4F1A 4F7F 7528 5F53 524D 7F16 8F91 5185 5BB9 3002"), Font = UiFont(8f), Padding = new Padding(4, 5, 0, 0) };
            targetFooter.Controls.Add(targetTip);
            targetEditor = new TextBox { Dock = DockStyle.Fill, Multiline = true, ScrollBars = ScrollBars.Vertical, AcceptsReturn = true, AcceptsTab = false, WordWrap = false, Font = new Font("Consolas", 10f), BackColor = Color.White, Text = LoadInitialTargets() };
            targetGroup.Controls.Add(targetEditor);
            targetEditor.BringToFront();

            var resultGroup = new GroupBox { Text = UiText.T("751F 6210 7ED3 679C 20 2D 20 5730 5740 548C 79C1 94A5"), Dock = DockStyle.Fill, Font = UiFont(9f, FontStyle.Bold) };
            mainSplit.Panel2.Controls.Add(resultGroup);
            resultGrid = new DataGridView { Dock = DockStyle.Fill, AllowUserToAddRows = false, AllowUserToDeleteRows = false, AllowUserToResizeRows = false, ReadOnly = true, RowHeadersVisible = false, SelectionMode = DataGridViewSelectionMode.CellSelect, MultiSelect = true, AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.Fill, BackgroundColor = Color.White, BorderStyle = BorderStyle.Fixed3D, Font = UiFont(9f), ClipboardCopyMode = DataGridViewClipboardCopyMode.EnableAlwaysIncludeHeaderText };
            resultGroup.Controls.Add(resultGrid);
            AddResultColumns();
            resultMenu = new ContextMenuStrip();
            resultMenu.Items.Add(UiText.T("590D 5236 9009 4E2D"), null, (sender, args) => CopySelectedResults());
            resultMenu.Items.Add(UiText.T("590D 5236 5168 90E8"), null, (sender, args) => CopyAllResults());
            resultGrid.ContextMenuStrip = resultMenu;
            resultGrid.KeyDown += (sender, args) =>
            {
                if (args.Control && args.KeyCode == Keys.C)
                {
                    CopySelectedResults();
                    args.Handled = true;
                }
            };

            pollTimer = new Timer { Interval = 500 };
            pollTimer.Tick += (sender, args) => ImportHitsFromFile(lastOutputPath);

            startButton.Click += (sender, args) => StartGeneration();
            stopButton.Click += (sender, args) => StopGeneration(false);
            pickOutputButton.Click += (sender, args) => PickOutput();
            openOutputButton.Click += (sender, args) => OpenOutputFile();
            copyButton.Click += (sender, args) => CopyAllResults();
            clearButton.Click += (sender, args) => ClearResults();
            saveTargetButton.Click += (sender, args) => SaveTargets();
            exitButton.Click += (sender, args) => Close();
            FormClosing += (sender, args) => { shuttingDown = true; StopGeneration(true); };
            Shown += (sender, args) => { splitLayoutReady = true; AlignMainSplit(); };
            SizeChanged += (sender, args) => AlignMainSplit();

            SetRunningState(false);
        }

        private static Font UiFont(float size, FontStyle style = FontStyle.Regular)
        {
            return new Font("Microsoft YaHei UI", size, style);
        }
        private static Button AddButton(Control parent, string text, int x, int y, int width) { var button = new Button { Text = text, Location = new Point(x, y), Size = new Size(width, 38), Font = UiFont(10f, FontStyle.Bold), UseVisualStyleBackColor = true }; parent.Controls.Add(button); return button; }
        private static Label AddLabel(Control parent, string text, int x, int y, int width) { var label = new Label { Text = text, Location = new Point(x, y), Size = new Size(width, 22), Font = UiFont(9f) }; parent.Controls.Add(label); return label; }
        private static Label AddValueLabel(Control parent, string text, int x, int y, int width) { var label = AddLabel(parent, text, x, y, width); label.Font = UiFont(9f, FontStyle.Bold); return label; }
        private static TextBox AddTextBox(Control parent, int x, int y, int width, string text) { var box = new TextBox { Location = new Point(x, y), Size = new Size(width, 24), Text = text, Font = UiFont(9f) }; parent.Controls.Add(box); return box; }
        private static NumericUpDown AddNumberBox(Control parent, int x, int y, int width, decimal min, decimal max, decimal value) { var box = new NumericUpDown { Location = new Point(x, y), Size = new Size(width, 24), Minimum = min, Maximum = max, Value = value, Font = UiFont(9f) }; parent.Controls.Add(box); return box; }

        private void AddResultColumns()
        {
            resultGrid.Columns.Add("No", UiText.T("5E8F 53F7"));
            resultGrid.Columns.Add("HitTime", UiText.T("65F6 95F4"));
            resultGrid.Columns.Add("Address", UiText.T("5730 5740"));
            resultGrid.Columns.Add("PrivateKey", UiText.T("79C1 94A5"));
            resultGrid.Columns.Add("Score", UiText.T("5206 6570"));
            resultGrid.Columns["No"].FillWeight = 34;
            resultGrid.Columns["HitTime"].FillWeight = 120;
            resultGrid.Columns["Address"].FillWeight = 280;
            resultGrid.Columns["PrivateKey"].FillWeight = 460;
            resultGrid.Columns["Score"].FillWeight = 50;
        }

        private string LoadInitialTargets()
        {
            if (File.Exists(defaultTargetsPath))
            {
                var text = File.ReadAllText(defaultTargetsPath, Encoding.UTF8).Trim();
                if (!string.IsNullOrWhiteSpace(text))
                {
                    return text;
                }
            }
            return "TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D";
        }

        private void SaveTargets()
        {
            var targets = GetTargets();
            if (targets.Count == 0)
            {
                AddLog(UiText.T("76EE 6807 5730 5740 5217 8868 4E3A 7A7A 3002"), true);
                return;
            }

            Directory.CreateDirectory(runtimeDir);
            File.WriteAllLines(defaultTargetsPath, targets, Encoding.UTF8);
            AddLog(UiText.T("76EE 6807 5730 5740 5DF2 4FDD 5B58 3002"));
        }

        private List<string> GetTargets()
        {
            return targetEditor.Lines.Select(line => line.Trim()).Where(line => line.Length > 0).ToList();
        }

        private string GetMatchingArgument()
        {
            var targets = GetTargets();
            if (targets.Count == 0) return string.Empty;
            if (targets.Count == 1) return targets[0];
            Directory.CreateDirectory(runtimeDir);
            File.WriteAllLines(defaultTargetsPath, targets, Encoding.UTF8);
            return defaultTargetsPath;
        }

        private void AddLog(string text, bool isError = false)
        {
            if (string.IsNullOrWhiteSpace(text)) return;
            logBox.AppendText((isError ? "[ERR] " : "") + text + Environment.NewLine);
            logBox.SelectionStart = logBox.TextLength;
            logBox.ScrollToCaret();
        }

        private void AlignMainSplit()
        {
            if (!splitLayoutReady || mainSplit == null || mainSplit.IsDisposed) return;
            mainSplit.Panel1MinSize = 300;
            mainSplit.Panel2MinSize = 720;
            var availableWidth = mainSplit.ClientSize.Width - mainSplit.SplitterWidth;
            if (availableWidth <= 0) return;
            var targetWidth = Math.Max(315, Math.Min(360, availableWidth / 4));
            if (availableWidth - targetWidth < mainSplit.Panel2MinSize)
            {
                targetWidth = Math.Max(mainSplit.Panel1MinSize, availableWidth - mainSplit.Panel2MinSize);
            }
            mainSplit.SplitterDistance = targetWidth;
        }

        private void PostToUi(Action action)
        {
            if (action == null || shuttingDown || IsDisposed || !IsHandleCreated) return;
            try
            {
                BeginInvoke(action);
            }
            catch
            {
            }
        }

        private void SetRunningState(bool running, string pid = "-")
        {
            statusValue.Text = running ? UiText.T("8FD0 884C 4E2D") : UiText.T("7A7A 95F2");
            statusValue.ForeColor = running ? Color.FromArgb(0, 110, 0) : Color.FromArgb(70, 70, 70);
            pidValue.Text = running ? pid : "-";
            startButton.Enabled = !running;
            stopButton.Enabled = running;
        }

        private void StartGeneration()
        {
            if (runningProcess != null)
            {
                try
                {
                    if (!runningProcess.HasExited)
                    {
                        AddLog(UiText.T("5DF2 6709 4EFB 52A1 5728 8FD0 884C 3002"), true);
                        return;
                    }
                }
                catch
                {
                }

                DisposeProcess(runningProcess);
                runningProcess = null;
                runningProcessId = 0;
            }

            if (!File.Exists(exePath))
            {
                AddLog(UiText.T("627E 4E0D 5230 53EF 6267 884C 6587 4EF6 3A 20") + exePath, true);
                return;
            }

            KillResidualGeneratorProcesses();

            var matching = GetMatchingArgument();
            if (string.IsNullOrWhiteSpace(matching))
            {
                AddLog(UiText.T("8BF7 5148 586B 5199 76EE 6807 5730 5740 3002"), true);
                return;
            }

            var outputPath = outputBox.Text.Trim();
            if (string.IsNullOrWhiteSpace(outputPath))
            {
                AddLog(UiText.T("8BF7 5148 9009 62E9 8F93 51FA 6587 4EF6 3002"), true);
                return;
            }

            var permanentOutputPath = outputPath;
            var permanentOutDir = Path.GetDirectoryName(permanentOutputPath);
            if (!string.IsNullOrWhiteSpace(permanentOutDir)) Directory.CreateDirectory(permanentOutDir);
            if (!File.Exists(permanentOutputPath))
            {
                File.WriteAllText(permanentOutputPath, string.Empty, Encoding.UTF8);
            }

            Directory.CreateDirectory(runOutputDir);
            lastOutputPath = CreateRunOutputPath();
            File.WriteAllText(lastOutputPath, string.Empty, Encoding.UTF8);

            seenHits.Clear();
            importedRunHits.Clear();
            hitCount = 0;
            hitCountValue.Text = "0";
            resultGrid.Rows.Clear();
            logBox.Clear();

            var arguments = BuildArguments(matching, lastOutputPath);
            AddLog(UiText.T("542F 52A8 547D 4EE4 3A 20") + "\"" + exePath + "\" " + arguments);
            AddLog("本次临时文件: " + lastOutputPath);
            AddLog("长期保存文件: " + permanentOutputPath);

            var startInfo = new ProcessStartInfo
            {
                FileName = exePath,
                Arguments = arguments,
                WorkingDirectory = rootDir,
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
                StandardOutputEncoding = Encoding.UTF8,
                StandardErrorEncoding = Encoding.UTF8
            };

            var proc = new Process { StartInfo = startInfo, EnableRaisingEvents = true };
            runningProcess = proc;
            runningProcessId = 0;

            proc.OutputDataReceived += (sender, args) => { if (args.Data != null) PostToUi(delegate { AddLog(args.Data); }); };
            proc.ErrorDataReceived += (sender, args) => { if (args.Data != null) PostToUi(delegate { AddLog(args.Data, true); }); };
            proc.Exited += (sender, args) =>
            {
                PostToUi(delegate
                {
                    if (!ReferenceEquals(runningProcess, proc))
                    {
                        DisposeProcess(proc);
                        return;
                    }

                    pollTimer.Stop();
                    ImportHitsFromFile(lastOutputPath);
                    try
                    {
                        AddLog(UiText.T("4EFB 52A1 7ED3 675F FF0C 9000 51FA 7801 3A 20") + proc.ExitCode);
                    }
                    catch
                    {
                        AddLog(UiText.T("4EFB 52A1 7ED3 675F 3002"));
                    }
                    SetRunningState(false);
                    runningProcess = null;
                    runningProcessId = 0;
                    DisposeProcess(proc);
                });
            };

            try
            {
                proc.Start();
                runningProcessId = proc.Id;
                AttachProcessToKillOnCloseJob(proc);
                proc.BeginOutputReadLine();
                proc.BeginErrorReadLine();
                SetRunningState(true, proc.Id.ToString());
                pollTimer.Start();
            }
            catch (Exception error)
            {
                AddLog(error.Message, true);
                if (runningProcessId > 0 || proc != null)
                {
                    KillProcessTreeAndWait(runningProcessId, proc, 10000);
                    KillResidualGeneratorProcesses();
                    DisposeProcess(proc);
                    CloseRunningJob();
                }
                SetRunningState(false);
                runningProcess = null;
                runningProcessId = 0;
            }
        }

        private string BuildArguments(string matching, string outputPath)
        {
            var args = new List<string>
            {
                "--matching", matching,
                "--prefix-count", ((int)prefixBox.Value).ToString(),
                "--suffix-count", ((int)suffixBox.Value).ToString(),
                "--quit-count", ((int)countBox.Value).ToString(),
                "--output", outputPath
            };
            return string.Join(" ", args.Select(QuoteArgument));
        }

        private string CreateRunOutputPath()
        {
            var stamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");
            return Path.Combine(runOutputDir, "hits_" + stamp + ".txt");
        }

        private static string QuoteArgument(string value)
        {
            if (value.IndexOfAny(new[] { ' ', '\t', '"' }) < 0) return value;
            return "\"" + value.Replace("\"", "\\\"") + "\"";
        }

        private void StopGeneration(bool force)
        {
            var proc = runningProcess;
            var pid = runningProcessId;
            if (proc == null)
            {
                KillResidualGeneratorProcesses();
                ImportHitsFromFile(lastOutputPath);
                pollTimer.Stop();
                SetRunningState(false);
                return;
            }

            AddLog(force ? UiText.T("6B63 5728 9000 51FA FF0C 6E05 7406 751F 6210 8FDB 7A0B 3002") : UiText.T("6B63 5728 505C 6B62 4EFB 52A1 FF0C 6E05 7406 751F 6210 8FDB 7A0B 3002"));

            try
            {
                if (!proc.HasExited)
                {
                    CloseRunningJob();
                    KillProcessTreeAndWait(pid, proc, 10000);
                }
            }
            catch { }

            CloseRunningJob();
            KillResidualGeneratorProcesses();
            ImportHitsFromFile(lastOutputPath);
            DisposeProcess(proc);

            pollTimer.Stop();
            runningProcess = null;
            runningProcessId = 0;
            SetRunningState(false);
        }

        private void PickOutput()
        {
            using (var dialog = new SaveFileDialog())
            {
                dialog.Title = UiText.T("9009 62E9 8F93 51FA 6587 4EF6");
                dialog.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
                dialog.FileName = outputBox.Text;
                if (dialog.ShowDialog(this) == DialogResult.OK)
                {
                    outputBox.Text = dialog.FileName;
                }
            }
        }

        private void OpenOutputFile()
        {
            var path = GetPermanentOutputPath();
            if (File.Exists(path))
            {
                Process.Start("notepad.exe", "\"" + path + "\"");
            }
            else
            {
                AddLog(UiText.T("8F93 51FA 6587 4EF6 4E0D 5B58 5728 3A 20") + path, true);
            }
        }

        private string GetPermanentOutputPath()
        {
            var path = outputBox.Text.Trim();
            return string.IsNullOrWhiteSpace(path) ? defaultOutputPath : path;
        }

        private void CopyAllResults()
        {
            var lines = new List<string>();
            foreach (DataGridViewRow row in resultGrid.Rows)
            {
                if (row.IsNewRow) continue;
                var address = Convert.ToString(row.Cells["Address"].Value);
                var privateKey = Convert.ToString(row.Cells["PrivateKey"].Value);
                if (!string.IsNullOrWhiteSpace(address) && !string.IsNullOrWhiteSpace(privateKey))
                {
                    lines.Add("address=" + address + " private=" + privateKey);
                }
            }

            if (lines.Count == 0)
            {
                AddLog(UiText.T("5F53 524D 6CA1 6709 53EF 590D 5236 7684 7ED3 679C 3002"), true);
                return;
            }

            Clipboard.SetText(string.Join(Environment.NewLine, lines));
            AddLog(UiText.T("7ED3 679C 5DF2 590D 5236 5230 526A 8D34 677F 3002"));
        }

        private void CopySelectedResults()
        {
            if (resultGrid.GetCellCount(DataGridViewElementStates.Selected) <= 0)
            {
                AddLog(UiText.T("8BF7 5148 9009 4E2D 7ED3 679C 3002"), true);
                return;
            }

            try
            {
                var text = resultGrid.GetClipboardContent();
                if (text != null)
                {
                    Clipboard.SetDataObject(text, true);
                    AddLog(UiText.T("9009 4E2D 7ED3 679C 5DF2 590D 5236 3002"));
                    return;
                }
            }
            catch
            {
            }

            var lines = new List<string>();
            foreach (DataGridViewCell cell in resultGrid.SelectedCells)
            {
                if (cell == null || cell.Value == null) continue;
                lines.Add(Convert.ToString(cell.Value));
            }

            if (lines.Count == 0)
            {
                AddLog(UiText.T("8BF7 5148 9009 4E2D 7ED3 679C 3002"), true);
                return;
            }

            Clipboard.SetText(string.Join(Environment.NewLine, lines));
            AddLog(UiText.T("9009 4E2D 7ED3 679C 5DF2 590D 5236 3002"));
        }

        private void ClearResults()
        {
            resultGrid.Rows.Clear();
            logBox.Clear();
            seenHits.Clear();
            hitCount = 0;
            hitCountValue.Text = "0";
        }

        private void ImportHitsFromFile(string path)
        {
            if (string.IsNullOrWhiteSpace(path) || !File.Exists(path)) return;
            foreach (var line in File.ReadLines(path, Encoding.UTF8))
            {
                var hit = ParseHit(line);
                if (hit == null) continue;
                var key = GetHitKey(hit);
                if (!importedRunHits.Add(key)) continue;
                AddHit(hit);
                AppendLineToPermanentOutput(line);
            }
        }

        private static Hit ParseHit(string line)
        {
            var match = Regex.Match(line.Trim(), @"^time=(.+?)\s+score=(\d+)\s+prefix=(\d+)\s+suffix=(\d+)\s+address=([1-9A-HJ-NP-Za-km-z]+)\s+private=([0-9a-fA-F]+)$");
            if (!match.Success) return null;
            return new Hit { Time = match.Groups[1].Value, Score = match.Groups[2].Value, Prefix = match.Groups[3].Value, Suffix = match.Groups[4].Value, Address = match.Groups[5].Value, PrivateKey = match.Groups[6].Value };
        }

        private void AddHit(Hit hit)
        {
            if (hit == null) return;
            var key = GetHitKey(hit);
            if (!seenHits.Add(key)) return;

            var rowIndex = resultGrid.Rows.Add();
            var row = resultGrid.Rows[rowIndex];
            row.Cells["No"].Value = rowIndex + 1;
            row.Cells["HitTime"].Value = hit.Time;
            row.Cells["Address"].Value = hit.Address;
            row.Cells["PrivateKey"].Value = hit.PrivateKey;
            row.Cells["Score"].Value = hit.Score;

            hitCount++;
            hitCountValue.Text = hitCount.ToString();
        }

        private static string GetHitKey(Hit hit)
        {
            return hit.Address + ":" + hit.PrivateKey;
        }

        private void AppendLineToPermanentOutput(string line)
        {
            var path = GetPermanentOutputPath();
            var dir = Path.GetDirectoryName(path);
            if (!string.IsNullOrWhiteSpace(dir))
            {
                Directory.CreateDirectory(dir);
            }
            File.AppendAllText(path, line + Environment.NewLine, Encoding.UTF8);
        }

        private void AttachProcessToKillOnCloseJob(Process process)
        {
            CloseRunningJob();

            try
            {
                var job = NativeMethods.CreateJobObject(IntPtr.Zero, null);
                if (job == IntPtr.Zero)
                {
                    return;
                }

                var info = new NativeMethods.JOBOBJECT_EXTENDED_LIMIT_INFORMATION();
                info.BasicLimitInformation.LimitFlags = NativeMethods.JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                var length = Marshal.SizeOf(typeof(NativeMethods.JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
                var infoPtr = Marshal.AllocHGlobal(length);
                try
                {
                    Marshal.StructureToPtr(info, infoPtr, false);
                    if (!NativeMethods.SetInformationJobObject(job, NativeMethods.JobObjectExtendedLimitInformation, infoPtr, (uint)length) ||
                        !NativeMethods.AssignProcessToJobObject(job, process.Handle))
                    {
                        NativeMethods.CloseHandle(job);
                        return;
                    }
                }
                finally
                {
                    Marshal.FreeHGlobal(infoPtr);
                }

                runningJobHandle = job;
            }
            catch
            {
                CloseRunningJob();
            }
        }

        private void CloseRunningJob()
        {
            if (runningJobHandle == IntPtr.Zero)
            {
                return;
            }

            try
            {
                NativeMethods.CloseHandle(runningJobHandle);
            }
            catch
            {
            }
            finally
            {
                runningJobHandle = IntPtr.Zero;
            }
        }

        private static bool IsProcessAlive(int pid)
        {
            if (pid <= 0)
            {
                return false;
            }

            try
            {
                using (var process = Process.GetProcessById(pid))
                {
                    return !process.HasExited;
                }
            }
            catch
            {
                return false;
            }
        }

        private void KillProcessTreeAndWait(int pid, Process proc, int timeoutMs)
        {
            if (pid > 0)
            {
                TryKillProcessTree(pid);
            }

            if (proc != null)
            {
                try
                {
                    if (!proc.HasExited)
                    {
                        proc.Kill();
                    }
                }
                catch
                {
                }

                try
                {
                    proc.WaitForExit(timeoutMs);
                }
                catch
                {
                }
            }

            if (IsProcessAlive(pid))
            {
                try
                {
                    using (var process = Process.GetProcessById(pid))
                    {
                        process.Kill();
                        process.WaitForExit(timeoutMs);
                    }
                }
                catch
                {
                }
            }
        }

        private static void DisposeProcess(Process process)
        {
            if (process == null)
            {
                return;
            }

            try
            {
                process.CancelOutputRead();
            }
            catch
            {
            }

            try
            {
                process.CancelErrorRead();
            }
            catch
            {
            }

            try
            {
                process.Dispose();
            }
            catch
            {
            }
        }

        private void TryKillProcessTree(int pid)
        {
            if (pid <= 0)
            {
                return;
            }

            try
            {
                using (var killer = new Process())
                {
                    killer.StartInfo = new ProcessStartInfo
                    {
                        FileName = "taskkill.exe",
                        Arguments = "/PID " + pid.ToString() + " /T /F",
                        UseShellExecute = false,
                        CreateNoWindow = true,
                        WindowStyle = ProcessWindowStyle.Hidden
                    };
                    killer.Start();
                    killer.WaitForExit(10000);
                }
            }
            catch
            {
            }
        }

        private void KillResidualGeneratorProcesses()
        {
            try
            {
                var name = Path.GetFileNameWithoutExtension(exePath);
                var normalizedExePath = Path.GetFullPath(exePath);
                var normalizedRoot = Path.GetFullPath(rootDir).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) + Path.DirectorySeparatorChar;
                foreach (var process in Process.GetProcesses())
                {
                    try
                    {
                        if (!process.ProcessName.StartsWith(name, StringComparison.OrdinalIgnoreCase) &&
                            !process.ProcessName.StartsWith("profanity", StringComparison.OrdinalIgnoreCase))
                        {
                            continue;
                        }

                        var fileName = string.Empty;
                        try
                        {
                            var module = process.MainModule;
                            if (module != null)
                            {
                                fileName = module.FileName ?? string.Empty;
                            }
                        }
                        catch { }

                        if (string.IsNullOrWhiteSpace(fileName))
                        {
                            continue;
                        }

                        var normalizedFileName = Path.GetFullPath(fileName);
                        var isThisGenerator = string.Equals(normalizedFileName, normalizedExePath, StringComparison.OrdinalIgnoreCase);
                        var isProjectGenerator = normalizedFileName.StartsWith(normalizedRoot, StringComparison.OrdinalIgnoreCase) &&
                            string.Equals(Path.GetFileName(normalizedFileName), "profanity.x64.exe", StringComparison.OrdinalIgnoreCase);

                        if (isThisGenerator || isProjectGenerator)
                        {
                            try
                            {
                                TryKillProcessTree(process.Id);
                                if (!process.HasExited)
                                {
                                    process.Kill();
                                }
                                process.WaitForExit(10000);
                            }
                            catch
                            {
                            }
                        }
                    }
                    catch
                    {
                    }
                }
            }
            catch { }
        }

        private sealed class Hit
        {
            public string Time;
            public string Score;
            public string Prefix;
            public string Suffix;
            public string Address;
            public string PrivateKey;
        }

        private static class NativeMethods
        {
            public const int JobObjectExtendedLimitInformation = 9;
            public const uint JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE = 0x00002000;

            [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
            public static extern IntPtr CreateJobObject(IntPtr lpJobAttributes, string lpName);

            [DllImport("kernel32.dll")]
            public static extern bool SetInformationJobObject(IntPtr hJob, int jobObjectInfoClass, IntPtr lpJobObjectInfo, uint cbJobObjectInfoLength);

            [DllImport("kernel32.dll")]
            public static extern bool AssignProcessToJobObject(IntPtr hJob, IntPtr hProcess);

            [DllImport("kernel32.dll")]
            public static extern bool CloseHandle(IntPtr hObject);

            [StructLayout(LayoutKind.Sequential)]
            public struct JOBOBJECT_BASIC_LIMIT_INFORMATION
            {
                public long PerProcessUserTimeLimit;
                public long PerJobUserTimeLimit;
                public uint LimitFlags;
                public UIntPtr MinimumWorkingSetSize;
                public UIntPtr MaximumWorkingSetSize;
                public uint ActiveProcessLimit;
                public UIntPtr Affinity;
                public uint PriorityClass;
                public uint SchedulingClass;
            }

            [StructLayout(LayoutKind.Sequential)]
            public struct IO_COUNTERS
            {
                public ulong ReadOperationCount;
                public ulong WriteOperationCount;
                public ulong OtherOperationCount;
                public ulong ReadTransferCount;
                public ulong WriteTransferCount;
                public ulong OtherTransferCount;
            }

            [StructLayout(LayoutKind.Sequential)]
            public struct JOBOBJECT_EXTENDED_LIMIT_INFORMATION
            {
                public JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInformation;
                public IO_COUNTERS IoInfo;
                public UIntPtr ProcessMemoryLimit;
                public UIntPtr JobMemoryLimit;
                public UIntPtr PeakProcessMemoryUsed;
                public UIntPtr PeakJobMemoryUsed;
            }
        }
    }
}
