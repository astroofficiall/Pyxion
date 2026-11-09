import "./globals.css"

export const metadata = {
  "title": "Dishscord"
}

export default function RootLayout({ children }: LayoutProps<"/">) {
  return (
    <html
      lang="en"
      className="h-full antialiased bg-black m-0xl"
    >
      <body className="min-h-screen min-w-screen m-[0px] bg-black">{children}</body>
    </html>
  );
}
