import resolve from '@rollup/plugin-node-resolve';
import commonjs from '@rollup/plugin-commonjs';
import typescript from '@rollup/plugin-typescript';
import babel from '@rollup/plugin-babel';
import peerDepsExternal from 'rollup-plugin-peer-deps-external';
import dts from 'rollup-plugin-dts';
import { readFileSync } from 'fs';

const packageJson = JSON.parse(readFileSync('./package.json', 'utf8'));

const external = [
  ...Object.keys(packageJson.peerDependencies || {}),
  ...Object.keys(packageJson.dependencies || {}),
  'react/jsx-runtime'
];

const commonPlugins = [
  peerDepsExternal(),
  resolve({
    browser: true,
    preferBuiltins: false
  }),
  commonjs(),
  babel({
    babelHelpers: 'bundled',
    exclude: 'node_modules/**',
    extensions: ['.js', '.jsx', '.ts', '.tsx'],
    presets: [
      ['@babel/preset-env', { targets: 'defaults' }],
      ['@babel/preset-react', { runtime: 'automatic' }],
      '@babel/preset-typescript'
    ]
  })
];

export default [
  // ESM and CommonJS builds
  {
    input: 'src/index.ts',
    output: [
      {
        file: packageJson.main,
        format: 'cjs',
        sourcemap: true,
        exports: 'named'
      },
      {
        file: packageJson.module,
        format: 'esm',
        sourcemap: true
      }
    ],
    external,
    plugins: [
      ...commonPlugins,
      typescript({
        tsconfig: './tsconfig.json',
        declaration: false,
        declarationMap: false
      })
    ]
  },
  // Type definitions
  {
    input: 'src/index.ts',
    output: {
      file: packageJson.types,
      format: 'esm'
    },
    external,
    plugins: [
      dts({
        tsconfig: './tsconfig.json'
      })
    ]
  }
];